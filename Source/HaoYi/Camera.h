
#pragma once

#include "HCNetSDK.h"

class CHaoYiView;
class CPushThread;
class CRecThread;
class CVideoWnd;
class CCamera
{
public:
	CCamera(CVideoWnd * lpWndParent);
	~CCamera(void);
public:
	string   &  GetDeviceSN() { return m_strDeviceSN; }
	CString  &	GetLogStatus() { return m_strLogStatus; }
public:
	BOOL		IsLogin();		// ���״ֻ̬��������ʾ����������״̬�ж�...
	BOOL		IsPlaying();	// �����������״̬�ľ�ȷ�ж�...
	BOOL		IsRecording() { return ((m_lpRecThread != NULL) ? true : false); }
	BOOL		IsPublishing() { return ((m_lpPushThread != NULL) ? true : false); }
	int			GetRecCourseID() { return m_nRecCourseID; }
	DWORD		GetHKErrCode() { return m_dwHKErrCode; }

	BOOL		InitCamera(GM_MapData & inMapLoc);
	GM_Error	ForUDPData(GM_MapData & inNetData);

	void		doRecEnd(int nRecSecond);

	DWORD		doSnapJPG(string & inUniqidName);
	DWORD		doPTZCmd(DWORD dwPTZCmd, BOOL bStop);
	DWORD		doLogin(HWND hWndNotify, LPCTSTR lpIPAddr, int nCmdPort, LPCTSTR lpUser, LPCTSTR lpPass);
	void		doLogout();

	void		UpdateWndTitle(CString & strTitle);
	void		ForLoginAsync(LONG lUserID, DWORD dwResult, LPNET_DVR_DEVICEINFO_V30 lpDeviceInfo);
	DWORD		onLoginSuccess();

	void		doRecStartCourse(int nCourseID);
	void		doRecStopCourse(int nCourseID);

	int			doStartLivePush(CHaoYiView * lpHaoYiView, int nPlayerSock, string & strRtmpUrl);
	void		doDelayTransmit(GM_Error inErr);
	void		doStopLiveMessage();
	void		doStopLivePush();
private:
	static void CALLBACK LoginResult(LONG lUserID, DWORD dwResult, LPNET_DVR_DEVICEINFO_V30 lpDeviceInfo, void * pUser);
private:
	void		WaitForExit();						// �ȴ��첽��¼�˳�...
	void		ClearResource();					// �ͷŽ�����Դ...

//	void		StartRecSlice();					// ����һ��¼����Ƭ...
//	void		doRecSlice();
private:
	int						m_nCameraID;			// ����ͷ���ر��...
	HWND					m_hWndNotify;			// ��Ϣ֪ͨ����...
	string					m_strDeviceSN;			// ��������ͷ���к�...
	CAMERA_TYPE				m_nCameraType;			// ��������ͷ����...
	GM_MapData				m_MapNetConfig;			// ����ͷ����������Ϣ...
	CVideoWnd		*		m_lpWndParent;			// �����ڶ���...
	CString					m_strLogStatus;			// ��¼״̬��...

	CRecThread		*		m_lpRecThread;			// ¼���̶߳���...
	CString					m_strMP4Name;			// MP4�ļ���(������չ��)...
	int						m_nRecCourseID;			// ����¼��Ŀγ̱��...

	CHaoYiView		*		m_lpHaoYiView;			// ������ͼ����...
	CPushThread     *		m_lpPushThread;			// ֱ���ϴ��߳�...
	string					m_strRtmpUrl;			// ֱ���ϴ���ַ...
	int						m_nPlayerSock;			// �����Ĳ�����...

	CString					m_strLoginUser;			// ��¼��¼�û�����...
	CString					m_strLoginPass;			// ��¼��½�û�����...
	int						m_nRtspPort;			// IPC��RTSP�˿ں�...

	LONG					m_dwHKErrCode;			// ��¼����������...
	BOOL					m_bIsExiting;			// ���ڵȴ��˳���...
	BOOL					m_HKLoginIng;			// �����첽��¼��...
	LONG					m_HKPlayID;				// ʵʱ���ű��...
	LONG					m_HKLoginID;			// ������¼���...
	NET_DVR_DEVICEINFO_V30	m_HKDeviceInfo;			// �����豸��Ϣ...
};