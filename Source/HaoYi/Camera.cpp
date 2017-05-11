
#include "StdAfx.h"
#include "Camera.h"
#include "UtilTool.h"
#include "VideoWnd.h"
#include "XmlConfig.h"
#include "RecThread.h"
#include "PushThread.h"
#include "HaoYiView.h"
#include "md5.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCamera::CCamera(CVideoWnd * lpWndParent)
  : m_lpWndParent(lpWndParent)
  , m_nCameraType(kCameraHK)
  , m_lpHaoYiView(NULL)
  , m_nRtspPort(554)
  , m_nPlayerSock(-1)
  , m_nRecCourseID(-1)
  , m_hWndNotify(NULL)
  , m_lpPushThread(NULL)
  , m_lpRecThread(NULL)
  , m_bIsExiting(false)
  , m_HKLoginIng(false)
  , m_dwHKErrCode(0)
  , m_HKLoginID(-1)
  , m_HKPlayID(-1)
  , m_nCameraID(0)
{
	ASSERT( m_lpWndParent != NULL );
	memset(&m_HKDeviceInfo, 0, sizeof(m_HKDeviceInfo));
}

CCamera::~CCamera(void)
{
	// �ȴ��첽��¼�˳�...
	this->WaitForExit();
	// �ͷ����е���Դ...
	this->ClearResource();
}
//
// �ȴ��첽��¼�˳� => ʹ�û��ⲻ������...
void CCamera::WaitForExit()
{
	m_bIsExiting = true;
	while( m_HKLoginIng ) {
		::Sleep(5);
	}
	ASSERT( !m_HKLoginIng );
}
//
// DVR�Ƿ��Ѿ��ɹ���½...
BOOL CCamera::IsLogin()
{
	if( m_nCameraType == kCameraHK ) {
		if( m_HKLoginID >= 0 ) {
			return true;
		} else {
			// �����첽��¼�У������¼�ɹ�...
			ASSERT( m_HKLoginID < 0 );
			return (m_HKLoginIng ? true : false);
		}
	} else if( m_nCameraType == kCameraDH ) {
		return false;
	}
	return false;
}
//
// DVR�Ƿ�����ʵʱԤ��...
BOOL CCamera::IsPlaying()
{
	switch( m_nCameraType )
	{
	case kCameraHK: return ((m_HKPlayID < 0) ? false : true);
	case kCameraDH: return false;
	}
	return false;
}
//
// �ͷŽ�����Դ...
void CCamera::ClearResource()
{
	// �ͷ�����¼����Դ��ʵʱԤ����Դ...
	if( m_HKPlayID >= 0 ) {
		NET_DVR_StopRealPlay(m_HKPlayID);
		m_HKPlayID = -1;
	}
	// �ͷŵ�¼��Դ...
	if( m_HKLoginID >= 0 ) {
		NET_DVR_Logout_V30(m_HKLoginID);
		m_HKLoginID = -1;
		memset(&m_HKDeviceInfo, 0, sizeof(m_HKDeviceInfo));
	}
	// �ͷ��Զ���rtsp¼�����...
	if( m_lpRecThread != NULL ) {
		delete m_lpRecThread;
		m_lpRecThread = NULL;
	}
	// �ͷ��Զ���ֱ���ϴ�����...
	this->doStopLivePush();
	// ����¼������Ϊ��Ч...
	m_nRecCourseID = -1;
}
//
// ������ͼ�¼�...
DWORD CCamera::doSnapJPG(string & inUniqidName)
{
	// Ϊ�˱��������һ��Ҫ�ò���״̬�жϣ���¼״̬�������첽�������᲻��ȷ...
	if( !this->IsPlaying() )
		return GM_NoErr;
	ASSERT( m_HKLoginID >= 0 );
	// DVR�ĵ�һ��ͨ����һ��DVR�����ж��ͨ��...
	DWORD dwReturn = 0;
	DWORD dwErr = GM_NoErr;
	LONG  nDvrStartChan = m_HKDeviceInfo.byStartChan;
	// �齨jpg�ṹ => ��ͼ��ѹ���ֱ���ΪVGAʱ��֧��0=CIF, 1=QCIF, 2=D1ץͼ, ���ֱ���Ϊ3=UXGA(1600x1200), 4=SVGA(800x600), 5=HD720p(1280x720),6=VGA,7=XVGA, 8=HD900p ��֧�ֵ�ǰ�ֱ��ʵ�ץͼ
	// Ĭ�����ó� D1 ģʽ�������������...
	NET_DVR_JPEGPARA dvrJpg = {0};
	dvrJpg.wPicSize = 2;		// D1
	dvrJpg.wPicQuality = 0;		// 0-��� 1-�Ϻ� 2-һ��
	// ��ȡ��ǰʱ��...
	CString strJPGName;
	GM_MapData theMapLoc;
	CXmlConfig & theConfig = CXmlConfig::GMInstance();
	theConfig.GetCamera(m_nCameraID, theMapLoc);
	string & strSavePath = theConfig.GetSavePath();
	string & strDBCameraID = theMapLoc["DBCameraID"];
	//strCurTime = CTime::GetCurrentTime().Format("%Y%m%d%H%M%S");
	//strJPGName.Format("%s\\%s_%d.jpg", strSavePath.c_str(), strCurTime, m_nCameraID);
	strJPGName.Format("%s\\%s_%s.jpg", strSavePath.c_str(), inUniqidName.c_str(), strDBCameraID.c_str());
	// ֱ����Ӳ��ץͼ���������Ƿ�����Ԥ��...
	if( !NET_DVR_CaptureJPEGPicture(m_HKLoginID, nDvrStartChan, &dvrJpg, (LPSTR)strJPGName.GetString()) ) {
		dwErr = NET_DVR_GetLastError();
		MsgLogGM(dwErr);
		return dwErr;
	}
	return GM_NoErr;
}
//
// ִ����̨����...
DWORD CCamera::doPTZCmd(DWORD dwPTZCmd, BOOL bStop)
{
	// ������Ԥ��ģʽ��ʹ��...
	DWORD dwErr = GM_NoErr;
	if( m_HKPlayID < 0 )
		return GM_NoErr;
	ASSERT( m_HKPlayID >= 0 );
	// ���ﲻ��Speed��ʽ���� PAN_AUTO �²�������...
	//bReturn = NET_DVR_PTZControlWithSpeed(m_HKPlayID, dwPTZCmd, bStop, nSpeed);
	//bReturn = NET_DVR_PTZControl_EX(m_HKPlayID, dwPTZCmd, bStop);
	if( !NET_DVR_PTZControl(m_HKPlayID, dwPTZCmd, bStop) ) {
		dwErr = NET_DVR_GetLastError();
		MsgLogGM(dwErr);
		return dwErr;
	}
	return GM_NoErr;
}
//
// ִ��DVRע������...
void CCamera::doLogout()
{
	// �ͷŽ�������Դ...
	this->ClearResource();
	// ��λ��¼״̬��Ϣ...
	GM_MapData   theMapLoc;
	CXmlConfig & theConfig = CXmlConfig::GMInstance();
	theConfig.GetCamera(m_nCameraID, theMapLoc);
	m_strLogStatus.Format("δ��¼ - %s", theMapLoc["Name"].c_str());
	// ������Ⱦ����״̬...
	ASSERT( m_lpWndParent != NULL );
	CRenderWnd * lpRenderWnd = m_lpWndParent->GetRenderWnd();
	lpRenderWnd->SetRenderState(CRenderWnd::ST_WAIT);
	lpRenderWnd->SetRenderText(m_strLogStatus);
}
//
// ��¼�ص������ӿ�...
void CALLBACK CCamera::LoginResult(LONG lUserID, DWORD dwResult, LPNET_DVR_DEVICEINFO_V30 lpDeviceInfo, void * pUser)
{
	CCamera * lpCamera = (CCamera*)pUser;
	lpCamera->ForLoginAsync(lUserID, dwResult, lpDeviceInfo);
}
//
// ʵ�ʴ�����¼�ص��ĺ���...
void CCamera::ForLoginAsync(LONG lUserID, DWORD dwResult, LPNET_DVR_DEVICEINFO_V30 lpDeviceInfo)
{
	// ���ڵ�¼״̬��λ...
	m_HKLoginIng = false;
	// �����˳�״̬�У�ֱ�ӷ���...
	if( m_bIsExiting ) {
		TRACE("CCamera::ForLoginAsync => Exit\n");
		return;
	}
	// ��¼ʧ�ܵĴ������̣���Ҫ֪ͨ�ϲ㴰��...
	if( dwResult <= 0 || lUserID < 0 ) {
		m_dwHKErrCode = NET_DVR_GetLastError();
		m_strLogStatus.Format("��¼ʧ�ܣ�����%s", NET_DVR_GetErrorMsg(&m_dwHKErrCode));
		m_lpWndParent->GetRenderWnd()->SetRenderText(m_strLogStatus);
		::PostMessage(m_hWndNotify, WM_DVR_LOGIN_RESULT, m_nCameraID, false);
		this->ClearResource();
		MsgLogGM(m_dwHKErrCode);
		return;
	}
	// ��¼�ɹ�֮�󣬱�������...
	m_dwHKErrCode = NET_DVR_NOERROR;
	this->m_HKLoginID = lUserID;
	memcpy(&m_HKDeviceInfo, lpDeviceInfo, sizeof(NET_DVR_DEVICEINFO_V30));
	// ��¼��½״̬��������ȷ���...
	GM_MapData   theMapLoc;
	CXmlConfig & theConfig = CXmlConfig::GMInstance();
	theConfig.GetCamera(m_nCameraID, theMapLoc);
	m_strLogStatus.Format("�ѵ�¼ - %s", theMapLoc["Name"].c_str());
	m_lpWndParent->GetRenderWnd()->SetRenderText(m_strLogStatus);
	// ����������״̬��֪ͨ�����ڣ������첽��¼���...
	ASSERT( m_hWndNotify != NULL && m_nCameraID > 0 );
	::PostMessage(m_hWndNotify, WM_DVR_LOGIN_RESULT, m_nCameraID, true);
}
//
// ִ��DVR�첽��¼�ɹ�����Ϣ�¼�...
DWORD CCamera::onLoginSuccess()
{
	ASSERT( m_nCameraID > 0 );
	DWORD dwErr = GM_NoErr;
	GM_MapData theMapLoc;
	CXmlConfig & theConfig = CXmlConfig::GMInstance();
	theConfig.GetCamera(m_nCameraID, theMapLoc);
	ASSERT( m_HKPlayID < 0 && m_HKLoginID >= 0 );
	do {
		// DVR�ĵ�һ��ͨ����һ��DVR�����ж��ͨ��...
		LONG  nDvrStartChan = m_HKDeviceInfo.byStartChan;
		DWORD dwReturn = 0;
		// ��ȡDVR��ͨ����Դ...
		//NET_DVR_IPPARACFG_V40 dvrIPCfg = {0};
		//BOOL bIPRet = NET_DVR_GetDVRConfig(m_HKLoginID, NET_DVR_GET_IPPARACFG_V40, nDvrStartChan, &dvrIPCfg, sizeof(dvrIPCfg), &dwReturn);
		// ��ȡ��̨������Ϣ...
		//NET_DVR_DECODERCFG_V30 dvrDecoderCfg = {0};
		//BOOL bDecRet = NET_DVR_GetDVRConfig(m_HKLoginID, NET_DVR_GET_DECODERCFG_V30, nDvrStartChan, &dvrDecoderCfg, sizeof(dvrDecoderCfg), &dwReturn);
		//(Ŀǰͨ��WEB����DVR) ��ȡ�豸���б������� => ��Ҫ����xml���ó����Զ�ѡ����û��ֶ�ѡ��д���豸����...
		//TCHAR szEncAbility[MAX_PATH*20] = {0};
		//if( !NET_DVR_GetDeviceAbility(m_HKLoginID, DEVICE_ENCODE_ALL_ABILITY, NULL, 0, szEncAbility, MAX_PATH*20) ) {
		//	dwErr = NET_DVR_GetLastError();
		//	MsgLogGM(dwErr);
		//	break;
		//}
		//TCHAR   szEncAbility[MAX_PATH*40] = {0};
		//TCHAR * lpInput = "<AudioVideoCompressInfo><AudioChannelNumber>1</AudioChannelNumber><VideoChannelNumber>1</VideoChannelNumber></AudioVideoCompressInfo>";
		//if( !NET_DVR_GetDeviceAbility(m_HKLoginID, DEVICE_ENCODE_ALL_ABILITY_V20, lpInput, strlen(lpInput), szEncAbility, MAX_PATH*40) ) {
		//	dwErr = NET_DVR_GetLastError();
		//	MsgLogGM(dwErr);
		//	break;
		//}
		// ��ȡIPC��rtsp�˿ں�...
		NET_DVR_RTSPCFG dvrRtsp = {0};
		if( !NET_DVR_GetRtspConfig(m_HKLoginID, 0, &dvrRtsp, sizeof(NET_DVR_RTSPCFG)) ) {
			dwErr = NET_DVR_GetLastError();
			MsgLogGM(dwErr);
			break;
		}
		// ������ȷ������rtsp�˿�...
		m_nRtspPort = dvrRtsp.wPort;
		// ��ȡ��Ϊ������������ => NET_DVR_GetDeviceConfig(NET_DVR_GET_MULTI_STREAM_COMPRESSIONCFG) => NET_DVR_MULTI_STREAM_COMPRESSIONCFG_COND NET_DVR_MULTI_STREAM_COMPRESSIONCFG
		// ��ȡѹ�����ò�����Ϣ => ������ ������ �� ������ ...
		NET_DVR_COMPRESSIONCFG_V30 dvrCompressCfg = {0};
		if( !NET_DVR_GetDVRConfig(m_HKLoginID, NET_DVR_GET_COMPRESSCFG_V30, nDvrStartChan, &dvrCompressCfg, sizeof(dvrCompressCfg), &dwReturn) ) {
			dwErr = NET_DVR_GetLastError();
			MsgLogGM(dwErr);
			break;
		}
		// �ж�������������������Ƶ�����Ƿ���ȷ => ������
		if( dvrCompressCfg.struNormHighRecordPara.byStreamType != 1 ||
			dvrCompressCfg.struNetPara.byStreamType != 1 ) {
			dwErr = GM_DVR_VType_Err;
			MsgLogGM(dwErr);
			break;
		}
		// �ж�����������������Ƶ���������Ƿ���ȷ => H264
		if( dvrCompressCfg.struNormHighRecordPara.byVideoEncType != NET_DVR_ENCODER_H264 ||
			dvrCompressCfg.struNetPara.byVideoEncType != NET_DVR_ENCODER_H264 ) {
			dwErr = GM_DVR_VEnc_Err;
			MsgLogGM(dwErr);
			break;
		}
		// �ж�����������������Ƶ���������Ƿ���ȷ => AAC
		if( dvrCompressCfg.struNormHighRecordPara.byAudioEncType != AUDIOTALKTYPE_AAC ||
			dvrCompressCfg.struNetPara.byAudioEncType != AUDIOTALKTYPE_AAC ) {
			dwErr = GM_DVR_AEnc_Err;
			MsgLogGM(dwErr);
			break;
		}
		// ����һЩ���� => �����豸�ı������������ã��������������õ�...
		// �������ļ��ж�ȡ��������������С����������С => �Զ�������...
		dvrCompressCfg.struNormHighRecordPara.dwVideoBitrate = theConfig.GetMainKbps() * 1024;
		dvrCompressCfg.struNetPara.dwVideoBitrate = theConfig.GetSubKbps() * 1024;
		dvrCompressCfg.struNormHighRecordPara.dwVideoBitrate |= 0x80000000;
		dvrCompressCfg.struNetPara.dwVideoBitrate |= 0x80000000;
		// ���� ������ �� ������ �����ò�����Ϣ...
		if( !NET_DVR_SetDVRConfig(m_HKLoginID, NET_DVR_SET_COMPRESSCFG_V30, nDvrStartChan, &dvrCompressCfg, sizeof(dvrCompressCfg)) ) {
			dwErr = NET_DVR_GetLastError();
			MsgLogGM(dwErr);
			break;
		}
		// ��ȡDVR�豸��ǰ�˲���...
		NET_DVR_CAMERAPARAMCFG dvrCCDParam = {0};
		if( !NET_DVR_GetDVRConfig(m_HKLoginID, NET_DVR_GET_CCDPARAMCFG, nDvrStartChan, &dvrCCDParam, sizeof(dvrCCDParam), &dwReturn) ) {
			dwErr = NET_DVR_GetLastError();
			MsgLogGM(dwErr);
			break;
		}
		// ��ͨ�������ļ��л�ȡ�Ƿ�������...
		string & strMirror = theMapLoc["OpenMirror"];
		BOOL bOpenMirror = ((strMirror.size() > 0) ? atoi(strMirror.c_str()) : false);
		// �Ծ���ģʽ���д��� => ����0 �ر�;1 ����;2 ����;3 �м�
		dvrCCDParam.byMirror = (bOpenMirror ? 3 : 0);
		if( !NET_DVR_SetDVRConfig(m_HKLoginID, NET_DVR_SET_CCDPARAMCFG, nDvrStartChan, &dvrCCDParam, sizeof(dvrCCDParam)) ) {
			dwErr = NET_DVR_GetLastError(); // ע���������ţ�NET_DVR_NETWORK_ERRORDATA
			MsgLogGM(dwErr);
			break;
		}
		// ��ȡͼ����� => OSD | ���� | ���� | ���� | ���� | ����
		NET_DVR_PICCFG_V40 dvrPicV40 = {0};
		if( !NET_DVR_GetDVRConfig(m_HKLoginID, NET_DVR_GET_PICCFG_V40, nDvrStartChan, &dvrPicV40, sizeof(dvrPicV40), &dwReturn) ) {
			dwErr = NET_DVR_GetLastError();
			MsgLogGM(dwErr);
			break;
		}
		// ��ͨ�������ļ��л�ȡ�Ƿ���OSD...
		string & strOSD = theMapLoc["OpenOSD"];
		BOOL bOpenOSD = ((strOSD.size() > 0) ? atoi(strOSD.c_str()) : true);
		// ����ͼ���ʽ => OSD | ���� | ���� | ���� | ���� | ����
		//strcpy((char*)dvrPicV40.sChanName, "Camera"); // ͨ������...
		//dvrPicV40.dwVideoFormat = 2; // ��Ƶ��ʽ��0- ��֧�֣�1- NTSC��2- PAL 
		//dvrPicV40.dwShowChanName = 0; // Ԥ����ͼ�����Ƿ���ʾͨ�����ƣ�0-����ʾ��1-��ʾ�������С704*576�� 
		//dvrPicV40.wShowNameTopLeftX = 200; // ͨ��������ʾλ�õ�x����
		//dvrPicV40.wShowNameTopLeftY = 100; // ͨ��������ʾλ�õ�y����
		//dvrPicV40.dwEnableHide = 1; // �Ƿ�������˽�ڱΣ�0-��1-��
		dvrPicV40.dwShowOsd = bOpenOSD; // Ԥ����ͼ�����Ƿ���ʾOSD��0-����ʾ��1-��ʾ�������С704*576��
		dvrPicV40.wOSDTopLeftX = 300; // OSD��x����
		dvrPicV40.wOSDTopLeftY = 20; // OSD��y����
		dvrPicV40.byOSDType = 2; // OSD����(�����ո�ʽ) 0��XXXX-XX-XX ������; 1��XX-XX-XXXX ������; 2��XXXX��XX��XX��; 3��XX��XX��XXXX��; 4��XX-XX-XXXX ������; 5��XX��XX��XXXX��; 6��xx/xx/xxxx ��/��/��; 7��xxxx/xx/xx ��/��/��; 8��xx/xx/xxxx ��/��/��
		dvrPicV40.byDispWeek = 0; // �Ƿ���ʾ���ڣ�0-����ʾ��1-��ʾ
		dvrPicV40.byOSDAttrib = 2; // OSD���ԣ�͸��/��˸����1��͸������˸��2��͸��������˸��3����˸����͸����4����͸��������˸
		dvrPicV40.byHourOSDType = 0; // Сʱ�ƣ�0��ʾ24Сʱ�ƣ�1��ʾ12Сʱ�ƻ�am/pm 
		dvrPicV40.byFontSize = 0xFF; // �����С��0- 16*16(��)/8*16(Ӣ)��1- 32*32(��)/16*32(Ӣ)��2- 64*64(��)/32*64(Ӣ)��3- 48*48(��)/24*48(Ӣ)��4- 24*24(��)/12*24(Ӣ)��5- 96*96(��)/48*96(Ӣ)��0xff- ����Ӧ
		dvrPicV40.byOSDColorType = 0; // OSD��ɫģʽ��0- Ĭ�ϣ��ڰף���1-�Զ���(��ɫ��struOsdColor)
		dvrPicV40.struOsdColor.byRed = 255;
		dvrPicV40.struOsdColor.byGreen = 0;
		dvrPicV40.struOsdColor.byBlue = 0;
		dvrPicV40.byAlignment = 0; // ���뷽ʽ��0- ����Ӧ��1- �Ҷ��룬2- �����
		if( !NET_DVR_SetDVRConfig(m_HKLoginID, NET_DVR_SET_PICCFG_V40, nDvrStartChan, &dvrPicV40, sizeof(dvrPicV40)) ) {
			dwErr = NET_DVR_GetLastError();
			MsgLogGM(dwErr);
			break;
		}
		// ��DVR�豸����Уʱ���� => ���óɸ�����ʱ��һ��...
		NET_DVR_TIME dvrTime = {0};
		CTime curTime = CTime::GetCurrentTime();
		dvrTime.dwYear = curTime.GetYear();
		dvrTime.dwMonth = curTime.GetMonth();
		dvrTime.dwDay = curTime.GetDay();
		dvrTime.dwHour = curTime.GetHour();
		dvrTime.dwMinute = curTime.GetMinute();
		dvrTime.dwSecond = curTime.GetSecond();
		if( !NET_DVR_SetDVRConfig(m_HKLoginID, NET_DVR_SET_TIMECFG, 0, &dvrTime, sizeof(dvrTime)) ) {
			dwErr = NET_DVR_GetLastError();
			MsgLogGM(dwErr);
			break;
		}
		// ׼����ʾԤ��������Ҫ�Ĳ���...
		ASSERT( m_lpWndParent != NULL );
		CRenderWnd * lpRenderWnd = m_lpWndParent->GetRenderWnd();
		NET_DVR_CLIENTINFO dvrClientInfo = {0};
		dvrClientInfo.hPlayWnd     = lpRenderWnd->m_hWnd;
		dvrClientInfo.lChannel     = nDvrStartChan;
		dvrClientInfo.lLinkMode    = 0;
		dvrClientInfo.sMultiCastIP = NULL;
		// ����ʵʱԤ���ӿ�...
		m_HKPlayID = NET_DVR_RealPlay_V30(m_HKLoginID, &dvrClientInfo, NULL, NULL, TRUE);
		if( m_HKPlayID < 0 ) {
			dwErr = NET_DVR_GetLastError();
			MsgLogGM(dwErr);
			break;
		}
		// ������Ⱦ����״̬...
		lpRenderWnd->SetRenderState(CRenderWnd::ST_RENDER);
	}while(false);
	// �������ʧ�ܣ����������Դ...
	if( dwErr != GM_NoErr ) {
		m_strLogStatus.Format("��¼ʧ�ܣ�����ţ�%lu", dwErr);
		m_lpWndParent->GetRenderWnd()->SetRenderText(m_strLogStatus);
		this->ClearResource();
		return dwErr;
	}
	// ��¼��½״̬��������ȷ���...
	m_strLogStatus.Format("�ѵ�¼ - %s", theMapLoc["Name"].c_str());
	m_lpWndParent->GetRenderWnd()->SetRenderText(m_strLogStatus);
	// ����һ��¼����Ƭ����...
	//this->StartRecSlice();
	return GM_NoErr;
}
//
// ����һ��¼����Ƭ...
/*void CCamera::StartRecSlice()
{
	// �����Զ����rtsp¼����� => �Զ�¼������������Ƶ...
	if( m_lpRecThread != NULL ) {
		delete m_lpRecThread;
		m_lpRecThread = NULL;
	}
	// ��ȡΨһ���ļ���...
	MD5	    md5;
	string  strUniqid;
	CString strTimeMicro;
	ULARGE_INTEGER	llTimCountCur = {0};
	::GetSystemTimeAsFileTime((FILETIME *)&llTimCountCur);
	strTimeMicro.Format("%I64d", llTimCountCur.QuadPart);
	md5.update(strTimeMicro, strTimeMicro.GetLength());
	strUniqid = md5.toString();

	// ����һ����ͼ�ļ�...
	this->doSnapJPG(strUniqid);

	// ��ȡ¼����Ƭʱ��(����)...
	GM_MapData theMapLoc;
	CXmlConfig & theConfig = CXmlConfig::GMInstance();
	DWORD dwRecSliceMS = theConfig.GetRecSlice() * 1000;
	theConfig.GetCamera(m_nCameraID, theMapLoc);
	// ׼��rtsp���ӵ�ַ��MP4¼������...
	CString  strRtspUrl, strMP4Path;
	string & strSavePath = theConfig.GetSavePath();
	string & strDBCameraID = theMapLoc["DBCameraID"];
	//strCurTime = CTime::GetCurrentTime().Format("%Y%m%d%H%M%S");
	//m_strMP4Name.Format("%s\\%s_%d", strSavePath.c_str(), strCurTime, m_nCameraID);
	m_strMP4Name.Format("%s\\%s_%s", strSavePath.c_str(), strUniqid.c_str(), strDBCameraID.c_str());
	strMP4Path.Format("%s.tmp", m_strMP4Name);
	// ׼��rtsp���ӵ�ַ => ������ => rtsp://admin:12345@192.168.1.65/Streaming/Channels/101
	strRtspUrl.Format("rtsp://%s:%s@%s:%d/Streaming/Channels/101", m_strLoginUser, m_strLoginPass, theMapLoc["IPv4Address"].c_str(), m_nRtspPort);
	// ����¼���̶߳���...
	ASSERT( m_lpWndParent != NULL && m_lpWndParent->m_hWnd != NULL );
	m_lpRecThread = new CRtspRecThread(m_lpWndParent->m_hWnd, this, dwRecSliceMS);
	// �����ɹ���ֱ�ӳ�ʼ��¼���߳�...
	if( !m_lpRecThread->InitThread(m_nCameraID, strRtspUrl, strMP4Path) ) {
		delete m_lpRecThread;
		m_lpRecThread = NULL;
	}
}*/
//
// ִ��¼����Ƭ�������������µ�¼��...
/*void CCamera::doRecSlice()
{
	// ���û�п�ʼ¼�ƣ�ֱ�ӷ���...
	if( m_lpRecThread == NULL )
		return;
	ASSERT( m_lpRecThread != NULL );
	// ֱ��ɾ��¼���߳�(����doRecEnd)�������µ�¼���߳�...
	delete m_lpRecThread; m_lpRecThread = NULL;
	// ������¼�����д��һ��ͳһ�ļ򵥺���...
	//this->StartRecSlice();
}*/
//
// ִ��¼������¼�...
void CCamera::doRecEnd(int nRecSecond)
{
	// �Ƚ��������еļ�¼��Ÿ�λ...
	m_nRecCourseID = -1;
	// ����¼���ļ�����չ��...
	CString strMP4Temp, strMP4File;
	strMP4Temp.Format("%s.tmp", m_strMP4Name);
	strMP4File.Format("%s_%d.mp4", m_strMP4Name, nRecSecond);
	// ����ļ������ڣ�ֱ�ӷ���...
	if( _access(strMP4Temp, 0) < 0 )
		return;
	// ֱ�Ӷ��ļ����и��Ĳ���������¼ʧ�ܵ���־...
	if( !MoveFile(strMP4Temp, strMP4File) ) {
		MsgLogGM(::GetLastError());
	}
}
//
// ��������¼�� => ͬһʱ��ֻ����һ��¼��...
void CCamera::doRecStartCourse(int nCourseID)
{
	// �����ǰͨ����û�в��ţ�ֱ�ӷ���...
	if( !this->IsPlaying() )
		return;
	// ����������еļ�¼���ǵ�ǰ��¼��ֱ�ӷ���...
	if( m_nRecCourseID == nCourseID )
		return;
	ASSERT( m_nRecCourseID != nCourseID );
	// ɾ��֮ǰ����¼��Ķ���...
	if( m_lpRecThread != NULL ) {
		delete m_lpRecThread;
		m_lpRecThread = NULL;
	}
	// ��ȡΨһ���ļ���...
	MD5	    md5;
	string  strUniqid;
	CString strTimeMicro;
	ULARGE_INTEGER	llTimCountCur = {0};
	::GetSystemTimeAsFileTime((FILETIME *)&llTimCountCur);
	strTimeMicro.Format("%I64d", llTimCountCur.QuadPart);
	md5.update(strTimeMicro, strTimeMicro.GetLength());
	strUniqid = md5.toString();
	// ����һ����ͼ�ļ� => �����ͼʧ�ܣ�ֱ�ӷ���...
	GM_Error theErr = GM_NoErr;
	theErr = this->doSnapJPG(strUniqid);
	if( theErr != GM_NoErr ) {
		MsgLogGM(theErr);
		return;
	}
	// ׼��¼����Ҫ����Ϣ...
	GM_MapData theMapLoc;
	CXmlConfig & theConfig = CXmlConfig::GMInstance();
	theConfig.GetCamera(m_nCameraID, theMapLoc);
	// ׼��rtsp���ӵ�ַ��MP4¼������ => PATH + Uniqid + DBCameraID + CourseID
	CString  strRtspUrl, strMP4Path;
	string & strSavePath = theConfig.GetSavePath();
	string & strDBCameraID = theMapLoc["DBCameraID"];
	m_strMP4Name.Format("%s\\%s_%s_%d", strSavePath.c_str(), strUniqid.c_str(), strDBCameraID.c_str(), nCourseID);
	strMP4Path.Format("%s.tmp", m_strMP4Name);
	// ׼��rtsp���ӵ�ַ => ������ => rtsp://admin:12345@192.168.1.65/Streaming/Channels/101
	strRtspUrl.Format("rtsp://%s:%s@%s:%d/Streaming/Channels/101", m_strLoginUser, m_strLoginPass, theMapLoc["IPv4Address"].c_str(), m_nRtspPort);
	// ����¼���̶߳���...
	ASSERT( m_lpWndParent != NULL && m_lpWndParent->m_hWnd != NULL );
	m_lpRecThread = new CRtspRecThread(m_lpWndParent->m_hWnd, this, 0);
	// �����ɹ���ֱ�ӳ�ʼ��¼���߳� => һ���������ɹ�...
	if( !m_lpRecThread->InitThread(nCourseID, strRtspUrl, strMP4Path) ) {
		delete m_lpRecThread;
		m_lpRecThread = NULL;
		return;
	}
	// ����¼��ȫ����ɣ����浱ǰ�γ̱��...
	m_nRecCourseID = nCourseID;
}
//
// ֹͣ����¼�� => ͬһʱ��ֻ����һ��¼��...
void CCamera::doRecStopCourse(int nCourseID)
{
	// �����ǰͨ����û�е�¼��ֱ�ӷ���...
	if( !this->IsPlaying() )
		return;
	// ����������еļ�¼���ǵ�ǰ��¼��ֱ�ӷ���...
	if( m_nRecCourseID != nCourseID )
		return;
	ASSERT( m_nRecCourseID == nCourseID );
	// ֱ��ɾ������¼����̶߳���...
	if( m_lpRecThread != NULL ) {
		delete m_lpRecThread;
		m_lpRecThread = NULL;
	}
	// ���������еļ�¼��Ÿ�λ...
	m_nRecCourseID = -1;
}
//
// ִ��DVR��¼����...
DWORD CCamera::doLogin(HWND hWndNotify, LPCTSTR lpIPAddr, int nCmdPort, LPCTSTR lpUser, LPCTSTR lpPass)
{
	// ��¼֮ǰ�����ͷ���Դ������֪ͨ����...
	DWORD dwErr = GM_NoErr;
	ASSERT( hWndNotify != NULL );
	m_hWndNotify = hWndNotify;
	this->ClearResource();
	// ��ʼ���ݲ�ͬ��DVR���ò�ͬ��SDK...
	if( m_nCameraType == kCameraHK ) {
		do {
			// �첽��ʽ��¼DVR�豸...
			NET_DVR_DEVICEINFO_V40  dvrDevV40 = {0};
			NET_DVR_USER_LOGIN_INFO dvrLoginInfo = {0};
			dvrLoginInfo.cbLoginResult = CCamera::LoginResult;
			strcpy(dvrLoginInfo.sDeviceAddress, lpIPAddr);
			strcpy(dvrLoginInfo.sUserName, lpUser);
			strcpy(dvrLoginInfo.sPassword, lpPass);
			dvrLoginInfo.bUseAsynLogin = 1;
			dvrLoginInfo.wPort = nCmdPort;
			dvrLoginInfo.pUser = this;
			// �����첽�ӿں���...
			if( NET_DVR_Login_V40(&dvrLoginInfo, &dvrDevV40) < 0 ) {
				dwErr = NET_DVR_GetLastError();
				MsgLogGM(dwErr);
				break;
			}
			// ���������첽��¼�б�־����¼�û���������...
			m_strLoginUser = lpUser;
			m_strLoginPass = lpPass;
			m_HKLoginIng = true;
		}while(false);
	} else if( m_nCameraType == kCameraDH ) {
	}
	// �������ʧ�ܣ����������Դ...
	if( dwErr != GM_NoErr ) {
		m_strLogStatus.Format("��¼ʧ�ܣ�����ţ�%lu", dwErr);
		m_lpWndParent->GetRenderWnd()->SetRenderText(m_strLogStatus);
		this->ClearResource();
		return dwErr;
	}
	// ��¼��½״̬��������ȷ���...
	GM_MapData   theMapLoc;
	CXmlConfig & theConfig = CXmlConfig::GMInstance();
	theConfig.GetCamera(m_nCameraID, theMapLoc);
	m_strLogStatus.Format("���ڵ�¼ - %s", theMapLoc["Name"].c_str());
	m_lpWndParent->GetRenderWnd()->SetRenderText(m_strLogStatus);
	return GM_NoErr;
}
//
// ��ʼ������ͷ...
BOOL CCamera::InitCamera(GM_MapData & inMapLoc)
{
	// ��������ͷ����...
	string & strType = inMapLoc["CameraType"];
	if( strType.size() > 0 ) {
		m_nCameraType = (CAMERA_TYPE)atoi(strType.c_str());
	}
	// �����豸���кţ������¼״̬...
	m_strDeviceSN = inMapLoc["DeviceSN"];
	m_strLogStatus.Format("δ��¼ - %s", inMapLoc["Name"].c_str());
	// �����ͨ�����ã�ֱ��д�������ļ�����...
	ASSERT( m_lpWndParent != NULL );
	m_nCameraID = m_lpWndParent->GetCameraID();
	CXmlConfig & theConfig = CXmlConfig::GMInstance();
	theConfig.SetCamera(m_nCameraID, inMapLoc);
	//return theConfig.GMSaveConfig();
	return true;
}
//
// ������ȡ���������������Ϣ...
GM_Error CCamera::ForUDPData(GM_MapData & inNetData)
{
	// ���е������������һ�£�ֱ�ӷ���...
	if( m_MapNetConfig == inNetData )
		return GM_NoErr;
	// ����Types�����治ͬ������ => inquiry | update...
	GM_Error theErr = GM_No_Command;
	string & strCmd = inNetData["Types"];
	// �ж��Ƿ�����Ч�������...
	if( strCmd.compare("inquiry") != 0 && strCmd.compare("update") != 0 ) {
		MsgLogGM(theErr);
		return theErr;
	}
	// ֱ�ӱ�����Ч���ݲ�����...
	ASSERT( m_MapNetConfig.size() >= 0 );
	m_MapNetConfig = inNetData;
	// ��ȡ��ǰ����ͷ��ŵ�����...
	ASSERT( m_lpWndParent != NULL );
	GM_MapData theMapLoc;
	int nCameraID = m_lpWndParent->GetCameraID();
	CXmlConfig & theConfig = CXmlConfig::GMInstance();
	theConfig.GetCamera(nCameraID, theMapLoc);
	ASSERT( theMapLoc.size() > 0 );
	// ���������ͷIP��ַ�����˸ı䣬֪ͨ�ϲ㴦����ͨ�������м��...
	//if( theMapLoc["IPv4Address"] != inNetData["IPv4Address"] ) {
	//}
	// �����Ѿ����ڵ�������Ϣ...
	theMapLoc["DeviceType"] = inNetData["DeviceType"];
	theMapLoc["DeviceDescription"] = inNetData["DeviceDescription"];
	theMapLoc["DeviceSN"] = inNetData["DeviceSN"];
	theMapLoc["CommandPort"] = inNetData["CommandPort"];
	theMapLoc["HttpPort"] = inNetData["HttpPort"];
	theMapLoc["MAC"] = inNetData["MAC"];
	theMapLoc["IPv4Address"] = inNetData["IPv4Address"];
	theMapLoc["BootTime"] = inNetData["BootTime"];
	theMapLoc["DigitalChannelNum"] = inNetData["DigitalChannelNum"];
	theMapLoc["DiskNumber"] = inNetData["DiskNumber"];
	// ����ȡ�ĸ�������ֱ�Ӵ���...
	theConfig.SetCamera(nCameraID, theMapLoc);
	theConfig.GMSaveConfig();
	return GM_NoErr;
}
//
// ���´��ڱ�������...
void CCamera::UpdateWndTitle(CString & strTitle)
{
	if( m_lpWndParent == NULL )
		return;
	// ����״̬��Ϣ���Ҳര��ʹ�ã�Ȼ����´��ڱ�����Ϣ...
	m_strLogStatus.Format("%s - %s", (this->IsLogin() ? "�ѵ�¼" : "δ��¼"), strTitle);
	m_lpWndParent->GetRenderWnd()->SetRenderText(m_strLogStatus);
	m_lpWndParent->SetTitleText(strTitle);
}
//
// ֹͣ�ϴ���ɾ���ϴ��߳�...
void CCamera::doStopLivePush()
{
	// ɾ���ϴ������߳�...
	if( m_lpPushThread != NULL ) {
		m_lpPushThread->m_bDeleteFlag = true;
		delete m_lpPushThread;
		m_lpPushThread = NULL;
	}
	// ����ʱת����Ϣ��λ...
	m_strRtmpUrl.clear();
	m_nPlayerSock = -1;
}
//
// ͨ����Ϣ�¼�ɾ���ϴ�ֱ��...
void CCamera::doStopLiveMessage()
{
	if( m_lpWndParent != NULL && m_lpWndParent->m_hWnd != NULL ) {
		::PostMessage(m_lpWndParent->m_hWnd, WM_ERR_PUSH_MSG, NULL, NULL);
	}
}
//
// ����ֱ���ϴ�...
// -1 => ����ʧ�ܣ����� err_code ...
//  0 => �Ѿ�������ֱ�ӷ�����������...
//  1 => �״���������Ҫ��ʱ������������...
int CCamera::doStartLivePush(CHaoYiView * lpHaoYiView, int nPlayerSock, string & strRtmpUrl)
{
	// �����ǰͨ����û�в��ţ�ֱ�ӷ���...
	if( !this->IsPlaying() )
		return -1;
	// ����ϴ�ͨ�������ϴ�����rtmp���ӷ��������Ŷ�...
	if( m_lpPushThread != NULL )
		return 0;
	ASSERT( m_lpPushThread == NULL );
	// ׼��ֱ����Ҫ������...
	CString strRtspUrl;
	GM_MapData theMapLoc;
	CXmlConfig & theConfig = CXmlConfig::GMInstance();
	theConfig.GetCamera(m_nCameraID, theMapLoc);
	// ׼��rtspֱ�����ӵ�ַ => ������ => rtsp://admin:12345@192.168.1.65/Streaming/Channels/102
	strRtspUrl.Format("rtsp://%s:%s@%s:%d/Streaming/Channels/102", m_strLoginUser, m_strLoginPass, theMapLoc["IPv4Address"].c_str(), m_nRtspPort);
	m_lpPushThread = new CPushThread(m_lpWndParent->m_hWnd);
	m_lpPushThread->InitThread(this, strRtspUrl, strRtmpUrl);
	// ����ǵ�һ���ϴ�����Ҫ��ʱ����...
	m_lpHaoYiView = lpHaoYiView;
	m_nPlayerSock = nPlayerSock;
	m_strRtmpUrl = strRtmpUrl;
	return 1;
}
//
// ��ʱֱ֪ͨ�����������ϴ����...
void CCamera::doDelayTransmit(GM_Error inErr)
{
	if( m_lpHaoYiView == NULL || m_nPlayerSock <= 0 )
		return;
	ASSERT( m_lpHaoYiView != NULL && m_nPlayerSock > 0 );
	// ��ʱת�����Ȼ�������ÿգ���λ...
	m_lpHaoYiView->doTransmitPlayer(m_nPlayerSock, m_strRtmpUrl, inErr);
	m_nPlayerSock = -1;
}