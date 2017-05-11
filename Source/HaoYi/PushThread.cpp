
#include "stdafx.h"
#include "PushThread.h"

#include "AmfByteStream.h"
#include "BitWritter.h"
#include "LibRtmp.h"
#include "Camera.h"

CRtspThread::CRtspThread()
{
	m_bFinished = false;
	m_strRtspUrl.clear();
	m_strAVCHeader.clear();
	m_strAACHeader.clear();
	m_lpPushThread = NULL;
	
	m_strPPS.clear();
	m_strSPS.clear();

	m_audio_rate_index = 0;
	m_audio_channel_num = 0;

	m_env_ = NULL;
	m_scheduler_ = NULL;
	m_rtspClient_ = NULL;

	m_rtspEventLoopWatchVariable = 0;
}

CRtspThread::~CRtspThread()
{
	// ����rtspѭ���˳���־...
	m_rtspEventLoopWatchVariable = 1;

	// ֹͣ�߳�...
	this->StopAndWaitForThread();

	TRACE("[~CRtspThread Thread] - Exit\n");
}

BOOL CRtspThread::InitRtsp(CPushThread * inPushThread, CString & strRtspUrl)
{
	// ���洫�ݵĲ���...
	m_lpPushThread = inPushThread;
	m_strRtspUrl = strRtspUrl.GetString();

	// ����rtsp���ӻ���...
	m_scheduler_ = BasicTaskScheduler::createNew();
	m_env_ = BasicUsageEnvironment::createNew(*m_scheduler_);
	m_rtspClient_ = ourRTSPClient::createNew(*m_env_, m_strRtspUrl.c_str(), 1, "rtspTransfer", this, NULL);
	
	// �����һ��rtsp����...
	m_rtspClient_->sendDescribeCommand(continueAfterDESCRIBE); 

	//����rtsp����߳�...
	this->Start();
	
	return true;
}

void CRtspThread::Entry()
{
	// ��������ѭ����⣬�޸� g_rtspEventLoopWatchVariable �����������˳�...
	ASSERT( m_env_ != NULL && m_rtspClient_ != NULL );
	m_env_->taskScheduler().doEventLoop(&m_rtspEventLoopWatchVariable);

	// �������ݽ�����־...
	m_bFinished = true;

	// ֪ͨ���̣߳���Ҫ�˳�...
	if( m_lpPushThread != NULL ) {
		m_lpPushThread->doErrNotify();
	}
	// �����˳�֮�����ͷ�rtsp�����Դ...
	// ֻ����������� shutdownStream() �����ط���Ҫ����...
	if( m_rtspClient_ != NULL ) {
		m_rtspClient_->shutdownStream();
		m_rtspClient_ = NULL;
	}

	// �ͷ��������...
	if( m_scheduler_ != NULL ) {
		delete m_scheduler_;
		m_scheduler_ = NULL;
	}

	// �ͷŻ�������...
	if( m_env_ != NULL ) {
		m_env_->reclaim();
		m_env_ = NULL;
	}
}

void CRtspThread::StartPushThread()
{
	if( m_lpPushThread == NULL )
		return;
	ASSERT( m_lpPushThread != NULL );
	// ������һ����Ƶ����Ƶ�Ѿ�׼������...
	ASSERT( m_strAACHeader.size() > 0 || m_strAVCHeader.size() > 0 );
	// ֱ������rtmp�����̣߳���ʼ����rtmp���͹���...
	m_lpPushThread->Start();
}

void CRtspThread::PushFrame(FMS_FRAME & inFrame)
{
	// ������֡��������Ͷ��е���...
	ASSERT( m_lpPushThread != NULL );
	m_lpPushThread->PushFrame(inFrame);
}

void CRtspThread::WriteAACSequenceHeader(int inAudioRate, int inAudioChannel)
{
	// ���Ƚ������洢���ݹ����Ĳ���...
	if (inAudioRate == 48000)
		m_audio_rate_index = 0x03;
	else if (inAudioRate == 44100)
		m_audio_rate_index = 0x04;
	else if (inAudioRate == 32000)
		m_audio_rate_index = 0x05;
	else if (inAudioRate == 24000)
		m_audio_rate_index = 0x06;
	else if (inAudioRate == 22050)
		m_audio_rate_index = 0x07;
	else if (inAudioRate == 16000)
		m_audio_rate_index = 0x08;
	else if (inAudioRate == 12000)
		m_audio_rate_index = 0x09;
	else if (inAudioRate == 11025)
		m_audio_rate_index = 0x0a;
	else if (inAudioRate == 8000)
		m_audio_rate_index = 0x0b;

	m_audio_channel_num = inAudioChannel;

	char   aac_seq_buf[4096] = {0};
    char * pbuf = aac_seq_buf;

    unsigned char flag = 0;
    flag = (10 << 4) |  // soundformat "10 == AAC"
        (3 << 2) |      // soundrate   "3  == 44-kHZ"
        (1 << 1) |      // soundsize   "1  == 16bit"
        1;              // soundtype   "1  == Stereo"

    pbuf = UI08ToBytes(pbuf, flag);
    pbuf = UI08ToBytes(pbuf, 0);    // aac packet type (0, header)

    // AudioSpecificConfig

	PutBitContext pb;
	init_put_bits(&pb, pbuf, 1024);
	put_bits(&pb, 5, 2);					//object type - AAC-LC
	put_bits(&pb, 4, m_audio_rate_index);	// sample rate index
	put_bits(&pb, 4, m_audio_channel_num);  // channel configuration

	//GASpecificConfig
	put_bits(&pb, 1, 0);    // frame length - 1024 samples
	put_bits(&pb, 1, 0);    // does not depend on core coder
	put_bits(&pb, 1, 0);    // is not extension

	flush_put_bits(&pb);
	pbuf += 2;

	// ����AAC����ͷ��Ϣ...
	int aac_len = (int)(pbuf - aac_seq_buf);
	m_strAACHeader.assign(aac_seq_buf, aac_len);
}

void CRtspThread::WriteAVCSequenceHeader(string & inSPS, string & inPPS)
{
	// �ȱ��� SPS �� PPS ��ʽͷ��Ϣ..
	ASSERT( inSPS.size() > 0 && inPPS.size() > 0 );
	m_strSPS = inSPS, m_strPPS = inPPS;

	// Write AVC Sequence Header use SPS and PPS...
	char * sps_ = (char*)m_strSPS.c_str();
	char * pps_ = (char*)m_strPPS.c_str();
	int    sps_size_ = m_strSPS.size();
	int	   pps_size_ = m_strPPS.size();

    char   avc_seq_buf[4096] = {0};
    char * pbuf = avc_seq_buf;

    unsigned char flag = 0;
    flag = (1 << 4) |				// frametype "1  == keyframe"
				 7;					// codecid   "7  == AVC"
    pbuf = UI08ToBytes(pbuf, flag);	// flag...
    pbuf = UI08ToBytes(pbuf, 0);    // avc packet type (0, header)
    pbuf = UI24ToBytes(pbuf, 0);    // composition time

    // AVC Decoder Configuration Record...

    pbuf = UI08ToBytes(pbuf, 1);            // configurationVersion
    pbuf = UI08ToBytes(pbuf, sps_[1]);      // AVCProfileIndication
    pbuf = UI08ToBytes(pbuf, sps_[2]);      // profile_compatibility
    pbuf = UI08ToBytes(pbuf, sps_[3]);      // AVCLevelIndication
    pbuf = UI08ToBytes(pbuf, 0xff);         // 6 bits reserved (111111) + 2 bits nal size length - 1
    pbuf = UI08ToBytes(pbuf, 0xe1);         // 3 bits reserved (111) + 5 bits number of sps (00001)
    pbuf = UI16ToBytes(pbuf, sps_size_);    // sps
    memcpy(pbuf, sps_, sps_size_);
    pbuf += sps_size_;
	
    pbuf = UI08ToBytes(pbuf, 1);            // number of pps
    pbuf = UI16ToBytes(pbuf, pps_size_);    // pps
    memcpy(pbuf, pps_, pps_size_);
    pbuf += pps_size_;

	// ����AVC����ͷ��Ϣ...
	int avc_len = (int)(pbuf - avc_seq_buf);
	m_strAVCHeader.assign(avc_seq_buf, avc_len);
}

CRtmpThread::CRtmpThread()
{
	m_bFinished = false;
	m_strRtmpUrl.clear();
	m_strAVCHeader.clear();
	m_strAACHeader.clear();
	m_lpPushThread = NULL;
	m_lpRtmp = NULL;
	
	m_strPPS.clear();
	m_strSPS.clear();

	m_audio_rate_index = 0;
	m_audio_channel_num = 0;
}

CRtmpThread::~CRtmpThread()
{
	// ֹͣ�߳�...
	this->StopAndWaitForThread();

	// ɾ��rtmp����...
	if( m_lpRtmp != NULL ) {
		delete m_lpRtmp;
		m_lpRtmp = NULL;
	}

	TRACE("[~CRtmpThread Thread] - Exit\n");
}

BOOL CRtmpThread::InitRtmp(CPushThread * inPushThread)
{
	// ���洫�ݵĲ���...
	m_lpPushThread = inPushThread;
	//m_nPushID = atoi(dbRec[CPushSession::GetItemName(kPushIDItem)].c_str());;
	//m_strRtmpUrl = dbRec[CPushSession::GetItemName(kPushRtspItem)];

	// ����rtmp����ע����������...
	m_lpRtmp = new LibRtmp(false, false, this, NULL);
	ASSERT( m_lpRtmp != NULL );

	//����rtmp����߳�...
	this->Start();
	
	return true;
}

void CRtmpThread::Entry()
{
	ASSERT( m_lpRtmp != NULL );
	ASSERT( m_lpPushThread != NULL );
	ASSERT( m_strRtmpUrl.size() > 0 );

	// ����rtmp������...
	if( !m_lpRtmp->Open(m_strRtmpUrl.c_str()) ) {
		delete m_lpRtmp; m_lpRtmp = NULL;
		m_lpPushThread->doErrNotify();
		m_bFinished = true;
		return;
	}
	// ѭ����ȡ���ݲ�ת����ȥ...
	while( !this->IsStopRequested() ) {
		if( m_lpRtmp->IsClosed() || !m_lpRtmp->Read() ) {
			m_lpPushThread->doErrNotify();
			m_bFinished = true;
			return;
		}
	}
	/*// �������ݽ�����־...
	m_bFinished = true;
	// ֪ͨ���̣߳���Ҫ�˳�...
	if( m_lpPushThread != NULL ) {
		m_lpPushThread->doErrNotify();
	}*/
}

void CRtmpThread::StartPushThread()
{
	if( m_lpPushThread == NULL )
		return;
	ASSERT( m_lpPushThread != NULL );
	// ������һ����Ƶ����Ƶ�Ѿ�׼������...
	ASSERT( m_strAACHeader.size() > 0 || m_strAVCHeader.size() > 0 );
	// ֱ������rtmp�����̣߳���ʼ����rtmp���͹���...
	m_lpPushThread->Start();
}

int CRtmpThread::PushFrame(FMS_FRAME & inFrame)
{
	// ������֡��������Ͷ��е���...
	ASSERT( m_lpPushThread != NULL );
	return m_lpPushThread->PushFrame(inFrame);
}

void CRtmpThread::WriteAACSequenceHeader(int inAudioRate, int inAudioChannel)
{
	// ���Ƚ������洢���ݹ����Ĳ���...
	if (inAudioRate == 48000)
		m_audio_rate_index = 0x03;
	else if (inAudioRate == 44100)
		m_audio_rate_index = 0x04;
	else if (inAudioRate == 32000)
		m_audio_rate_index = 0x05;
	else if (inAudioRate == 24000)
		m_audio_rate_index = 0x06;
	else if (inAudioRate == 22050)
		m_audio_rate_index = 0x07;
	else if (inAudioRate == 16000)
		m_audio_rate_index = 0x08;
	else if (inAudioRate == 12000)
		m_audio_rate_index = 0x09;
	else if (inAudioRate == 11025)
		m_audio_rate_index = 0x0a;
	else if (inAudioRate == 8000)
		m_audio_rate_index = 0x0b;

	m_audio_channel_num = inAudioChannel;

	char   aac_seq_buf[4096] = {0};
    char * pbuf = aac_seq_buf;

    unsigned char flag = 0;
    flag = (10 << 4) |  // soundformat "10 == AAC"
        (3 << 2) |      // soundrate   "3  == 44-kHZ"
        (1 << 1) |      // soundsize   "1  == 16bit"
        1;              // soundtype   "1  == Stereo"

    pbuf = UI08ToBytes(pbuf, flag);
    pbuf = UI08ToBytes(pbuf, 0);    // aac packet type (0, header)

    // AudioSpecificConfig

	PutBitContext pb;
	init_put_bits(&pb, pbuf, 1024);
	put_bits(&pb, 5, 2);					//object type - AAC-LC
	put_bits(&pb, 4, m_audio_rate_index);	// sample rate index
	put_bits(&pb, 4, m_audio_channel_num);  // channel configuration

	//GASpecificConfig
	put_bits(&pb, 1, 0);    // frame length - 1024 samples
	put_bits(&pb, 1, 0);    // does not depend on core coder
	put_bits(&pb, 1, 0);    // is not extension

	flush_put_bits(&pb);
	pbuf += 2;

	// ����AAC����ͷ��Ϣ...
	int aac_len = (int)(pbuf - aac_seq_buf);
	m_strAACHeader.assign(aac_seq_buf, aac_len);
}

void CRtmpThread::WriteAVCSequenceHeader(string & inSPS, string & inPPS)
{
	// �ȱ��� SPS �� PPS ��ʽͷ��Ϣ..
	ASSERT( inSPS.size() > 0 && inPPS.size() > 0 );
	m_strSPS = inSPS, m_strPPS = inPPS;

	// Write AVC Sequence Header use SPS and PPS...
	char * sps_ = (char*)m_strSPS.c_str();
	char * pps_ = (char*)m_strPPS.c_str();
	int    sps_size_ = m_strSPS.size();
	int	   pps_size_ = m_strPPS.size();

    char   avc_seq_buf[4096] = {0};
    char * pbuf = avc_seq_buf;

    unsigned char flag = 0;
    flag = (1 << 4) |				// frametype "1  == keyframe"
				 7;					// codecid   "7  == AVC"
    pbuf = UI08ToBytes(pbuf, flag);	// flag...
    pbuf = UI08ToBytes(pbuf, 0);    // avc packet type (0, header)
    pbuf = UI24ToBytes(pbuf, 0);    // composition time

    // AVC Decoder Configuration Record...

    pbuf = UI08ToBytes(pbuf, 1);            // configurationVersion
    pbuf = UI08ToBytes(pbuf, sps_[1]);      // AVCProfileIndication
    pbuf = UI08ToBytes(pbuf, sps_[2]);      // profile_compatibility
    pbuf = UI08ToBytes(pbuf, sps_[3]);      // AVCLevelIndication
    pbuf = UI08ToBytes(pbuf, 0xff);         // 6 bits reserved (111111) + 2 bits nal size length - 1
    pbuf = UI08ToBytes(pbuf, 0xe1);         // 3 bits reserved (111) + 5 bits number of sps (00001)
    pbuf = UI16ToBytes(pbuf, sps_size_);    // sps
    memcpy(pbuf, sps_, sps_size_);
    pbuf += sps_size_;
	
    pbuf = UI08ToBytes(pbuf, 1);            // number of pps
    pbuf = UI16ToBytes(pbuf, pps_size_);    // pps
    memcpy(pbuf, pps_, pps_size_);
    pbuf += pps_size_;

	// ����AVC����ͷ��Ϣ...
	int avc_len = (int)(pbuf - avc_seq_buf);
	m_strAVCHeader.assign(avc_seq_buf, avc_len);
}

CPushThread::CPushThread(HWND hWndParent)
  : m_hWndParent(hWndParent)
{
	ASSERT( m_hWndParent != NULL );

	m_lpRtmp = NULL;
	m_lpRtspThread = NULL;
	m_lpRtmpThread = NULL;
	m_dwTimeOutMS = 0;
	m_bDeleteFlag = false;

	m_bFileMode = false;
	m_strRtmpUrl.clear();
	m_nCurSendByte = 0;
	m_nSendKbps = 0;
}

CPushThread::~CPushThread()
{
	// ֹͣ�߳�...
	this->StopAndWaitForThread();

	// ɾ��rtmp�����������ӻ��⣬����Connect����ʱrtmp�Ѿ���ɾ��������ڴ����...
	if( m_lpRtmp != NULL ) {
		OSMutexLocker theLock(&m_Mutex);
		delete m_lpRtmp; m_lpRtmp = NULL;
	}

	// ɾ��rtmp�߳�...
	if( m_lpRtmpThread != NULL ) {
		delete m_lpRtmpThread;
		m_lpRtmpThread = NULL;
	}

	// ɾ��rtsp�߳�...
	if( m_lpRtspThread != NULL ) {
		delete m_lpRtspThread;
		m_lpRtspThread = NULL;
	}
	
	TRACE("[~CPushThread Thread] - Exit\n");
}

BOOL CPushThread::InitThread(CCamera * lpCamera, CString & strRtspUrl, string & strRtmpUrl)
{
	// ���洫�ݹ����Ĳ���...
	m_lpCamera = lpCamera;
	m_strRtmpUrl = strRtmpUrl;
	// ����rtsp���������߳�...
	m_lpRtspThread = new CRtspThread();
	m_lpRtspThread->InitRtsp(this, strRtspUrl);

	// ����rtsp��rtmp�߳�...
	/*if( strnicmp("rtsp://", strRtspUrl.c_str(), strlen("rtsp://")) == 0 ) {
		m_lpRtspThread = new CRtspThread();
		m_lpRtspThread->InitRtsp(this);
	} else {
		m_lpRtmpThread = new CRtmpThread();
		m_lpRtmpThread->InitRtmp(this);
	}*/
	
	// ����rtmp�ϴ�����...
	m_lpRtmp = new LibRtmp(false, true, NULL, NULL);
	ASSERT( m_lpRtmp != NULL );

	// ���ﲻ�������̣߳��ȴ�����׼���׵�֮���������...

	return true;
}

bool CPushThread::IsDataFinished()
{
	if( m_lpRtspThread != NULL ) {
		return m_lpRtspThread->IsFinished();
	} else if( m_lpRtmpThread != NULL ) {
		return m_lpRtmpThread->IsFinished();
	}
	return true;
}

void CPushThread::doErrNotify()
{
	// ������Ҫ֪ͨ���ڵȴ��Ĳ������˳�...
	if( m_lpCamera != NULL ) {
		m_lpCamera->doDelayTransmit(GM_Push_Fail);
	}
	// ����߳��Ѿ�������ɾ���ˣ�ֱ���˳�...
	if( m_bDeleteFlag )
		return;
	ASSERT( !m_bDeleteFlag );
	ASSERT( m_hWndParent != NULL );
	::PostMessage(m_hWndParent, WM_ERR_PUSH_MSG, NULL, NULL);
}

void CPushThread::Entry()
{
	// ����rtmp server��ʧ�ܣ�֪ͨ�ϲ�ɾ��֮...
	if( !this->OpenRtmpUrl() ) {
		TRACE("[CPushThread::OpenRtmpUrl] - Error\n");
		this->doErrNotify();
		return;
	}
	// ���ֳɹ�������metadata���ݰ���ʧ�ܣ�֪ͨ�ϲ�ɾ��֮...
	if( !this->SendMetadataPacket() ) {
		TRACE("[CPushThread::SendMetadataPacket] - Error\n");
		this->doErrNotify();
		return;
	}
	// ������Ƶ����ͷ���ݰ���ʧ�ܣ�֪ͨ�ϲ�ɾ��֮...
	if( !this->SendAVCSequenceHeaderPacket() ) {
		TRACE("[CPushThread::SendAVCSequenceHeaderPacket] - Error\n");
		this->doErrNotify();
		return;
	}
	// ������Ƶ����ͷ���ݰ���ʧ�ܣ�֪ͨ�ϲ�ɾ��֮...
	if( !this->SendAACSequenceHeaderPacket() ) {
		TRACE("[CPushThread::SendAACSequenceHeaderPacket] - Error\n");
		this->doErrNotify();
		return;
	}
	// �ϴ�׼����ϣ�֪ͨ RemoteSession ���Է�����Ϣ��ֱ����������...
	if( m_lpCamera != NULL ) {
		m_lpCamera->doDelayTransmit(GM_NoErr);
	}
	// ��ʱʱ���¼���...
	m_dwTimeOutMS = ::GetTickCount();
	// ��ʼ�߳�ѭ��...
	int nRetValue = 0;
	uint32_t dwStartTimeMS = ::GetTickCount();
	while( !this->IsStopRequested() ) {
		// ���ʱ����������1000���룬�����һ�η�������...
		if( (::GetTickCount() - dwStartTimeMS) >= 1000 ) {
			dwStartTimeMS = ::GetTickCount();
			m_nSendKbps = m_nCurSendByte * 8 / 1024;
			m_nCurSendByte = 0;
		}
		nRetValue = this->SendOneDataPacket();
		// < 0 ֱ�����ϲ㷴��ɾ��֮...
		if( nRetValue < 0 ) {
			TRACE("[CPushThread::SendOneDataPacket] - Error\n");
			this->doErrNotify();
			return;
		}
		// == 0 ���ϼ���...
		if( nRetValue == 0 ) {
			Sleep(5);
			continue;
		}
		// > 0 ˵������,���Ϸ���...
		ASSERT( nRetValue > 0 );
	}
}

BOOL CPushThread::OpenRtmpUrl()
{
	OSMutexLocker theLock(&m_Mutex);
	if( m_lpRtmp == NULL || m_strRtmpUrl.size() <= 0  )
		return false;
	ASSERT( m_lpRtmp != NULL && m_strRtmpUrl.size() > 0);
	
	// ����rtmp server��������ֵ�Э��
	return m_lpRtmp->Open(m_strRtmpUrl.c_str());
}
//
// ͨ������������������֡...
int CPushThread::PushFrame(FMS_FRAME & inFrame)
{
	OSMutexLocker theLock(&m_Mutex);

	m_MapFrame.insert(pair<uint32_t, FMS_FRAME>(inFrame.dwSendTime, inFrame));

	return m_MapFrame.size();
}
//
// ������ݰ��Ƿ��Ѿ���ʱ...
BOOL CPushThread::IsFrameTimeout()
{
	// �����ж������߳��Ƿ��Ѿ�������...
	if( this->IsDataFinished() )
		return true;
	// ������治����100�����ݰ�����ʱʧ�ܣ����ó�ʱ��ʱ��...
	if( m_MapFrame.size() >= 100 ) {
		m_dwTimeOutMS = ::GetTickCount();
		return false;
	}
	ASSERT( m_MapFrame.size() < 100 );
	// ���ݰ�С��100����������ʱ��ֱ������Ԥ��ʱ��(3����)...
	int   nWaitMinute = 3;
	DWORD dwElapseSec = (::GetTickCount() - m_dwTimeOutMS) / 1000;
	return ((dwElapseSec >= nWaitMinute * 60) ? true : false);
}
//
// ����һ��֡���ݰ�...
uint32_t g_LastATime = 0;
int CPushThread::SendOneDataPacket()
{
	OSMutexLocker theLock(&m_Mutex);

	// �����ʱ�����һ�����Ļ��壬����Ϊ��Ҫ�Ͽ�������...
	if( this->IsFrameTimeout() ) {
		TRACE("[CPushThread::FrameTimeout] - Frame(%d)\n", m_MapFrame.size());
		return -1;
	}
	// ������ݻ�û�н���������Ҫ��һ�����棬�Ա�����Ƶ�ܹ��Զ�����Ȼ���ٷ������ݰ�...
	if( !this->IsDataFinished() && m_MapFrame.size() < 100 ) {
		return 0;
	}
	ASSERT( !this->IsDataFinished() || m_MapFrame.size() >= 100 );

	bool is_ok = false;
	ASSERT( m_MapFrame.size() > 0 );
	
	KH_MapFrame::iterator itorFrame;
	KH_MapFrame::iterator itorPair;
	pair<KH_MapFrame::iterator, KH_MapFrame::iterator> myPair;

	// ��ȡ����ͷ�����ҵ��ظ����б�...
	itorFrame = m_MapFrame.begin();
	myPair = m_MapFrame.equal_range(itorFrame->first);

	for(itorPair = myPair.first; itorPair != myPair.second; ++itorPair) {
		// ���������ͬʱ����б�...
		ASSERT( itorPair->first == itorFrame->first );
		FMS_FRAME & theFrame = itorPair->second;
		// ����ȡ��������֡������...
		switch( theFrame.typeFlvTag )
		{
		case FLV_TAG_TYPE_AUDIO: is_ok = this->SendAudioDataPacket(theFrame); break;
		case FLV_TAG_TYPE_VIDEO: is_ok = this->SendVideoDataPacket(theFrame); break;
		}
		// �ۼӷ����ֽ���...
		m_nCurSendByte += theFrame.strData.size();
		// ��ӡ���ͽ��...
		if( theFrame.typeFlvTag == FLV_TAG_TYPE_AUDIO ) {
			g_LastATime = ((g_LastATime <= 0 ) ? theFrame.dwSendTime: g_LastATime);
			//TRACE("[%s] SendTime = %lu, Duration = %lu, Size = %d, MapSize = %d\n", ((theFrame.typeFlvTag == FLV_TAG_TYPE_VIDEO) ? "Video" : "Audio"), theFrame.dwSendTime, theFrame.dwSendTime - g_LastATime, theFrame.strData.size(), m_MapFrame.size());
			g_LastATime = theFrame.dwSendTime;
		}
	}

	// �Ӷ������Ƴ�һ����ͬʱ������ݰ� => һ��Ҫ�ùؼ���ɾ��������ɾ���ɾ�...
	m_MapFrame.erase(itorFrame->first);

	// ����ʧ�ܣ����ش���...
	return (is_ok ? 1 : -1);
}
//
// ������Ƶ���ݰ�...
BOOL CPushThread::SendVideoDataPacket(FMS_FRAME & inFrame)
{
    int need_buf_size = inFrame.strData.size() + 5;
	char * video_mem_buf_ = new char[need_buf_size];
    char * pbuf  = video_mem_buf_;

	bool   is_ok = false;
    unsigned char flag = 0;
    flag = (inFrame.is_keyframe ? 0x17 : 0x27);

    pbuf = UI08ToBytes(pbuf, flag);
    pbuf = UI08ToBytes(pbuf, 1);    // avc packet type (0, nalu)
    pbuf = UI24ToBytes(pbuf, 0);    // composition time

    memcpy(pbuf, inFrame.strData.c_str(), inFrame.strData.size());
    pbuf += inFrame.strData.size();

    is_ok = m_lpRtmp->Send(video_mem_buf_, (int)(pbuf - video_mem_buf_), FLV_TAG_TYPE_VIDEO, inFrame.dwSendTime);

	delete [] video_mem_buf_;

	//TRACE("[Video] SendTime = %lu, StartTime = %lu\n", inFrame.dwSendTime, inFrame.dwStartTime);

	return is_ok;
}
//
// ������Ƶ���ݰ�...
BOOL CPushThread::SendAudioDataPacket(FMS_FRAME & inFrame)
{
    int need_buf_size = inFrame.strData.size() + 5;
	char * audio_mem_buf_ = new char[need_buf_size];
    char * pbuf  = audio_mem_buf_;

	bool   is_ok = false;
    unsigned char flag = 0;
    flag = (10 << 4) |  // soundformat "10 == AAC"
        (3 << 2) |      // soundrate   "3  == 44-kHZ"
        (1 << 1) |      // soundsize   "1  == 16bit"
        1;              // soundtype   "1  == Stereo"

    pbuf = UI08ToBytes(pbuf, flag);
    pbuf = UI08ToBytes(pbuf, 1);    // aac packet type (1, raw)

    memcpy(pbuf, inFrame.strData.c_str(), inFrame.strData.size());
    pbuf += inFrame.strData.size();

    is_ok = m_lpRtmp->Send(audio_mem_buf_, (int)(pbuf - audio_mem_buf_), FLV_TAG_TYPE_AUDIO, inFrame.dwSendTime);

	delete [] audio_mem_buf_;
	
	//TRACE("[Audio] SendTime = %lu, StartTime = %lu\n", inFrame.dwSendTime, inFrame.dwStartTime);

	return is_ok;
}
//
// ������Ƶ����ͷ...
BOOL CPushThread::SendAVCSequenceHeaderPacket()
{
	OSMutexLocker theLock(&m_Mutex);

	string strAVC;

	if( m_lpRtspThread != NULL ) {
		strAVC = m_lpRtspThread->GetAVCHeader();
	} else if( m_lpRtmpThread != NULL ) {
		strAVC = m_lpRtmpThread->GetAVCHeader();
	}

	// û����Ƶ����...
	if( strAVC.size() <= 0 )
		return true;
	
	ASSERT( strAVC.size() > 0 );
	return m_lpRtmp->Send(strAVC.c_str(), strAVC.size(), FLV_TAG_TYPE_VIDEO, 0);
}

//
// ������Ƶ����ͷ...
BOOL CPushThread::SendAACSequenceHeaderPacket()
{
	OSMutexLocker theLock(&m_Mutex);

	string strAAC;

	if( m_lpRtspThread != NULL ) {
		strAAC = m_lpRtspThread->GetAACHeader();
	} else if( m_lpRtmpThread != NULL ) {
		strAAC = m_lpRtmpThread->GetAACHeader();
	}

	// û����Ƶ����...
	if( strAAC.size() <= 0 )
		return true;

	ASSERT( strAAC.size() > 0 );
    return m_lpRtmp->Send(strAAC.c_str(), strAAC.size(), FLV_TAG_TYPE_AUDIO, 0);
}

BOOL CPushThread::SendMetadataPacket()
{
	OSMutexLocker theLock(&m_Mutex);

	char   metadata_buf[4096];
    char * pbuf = this->WriteMetadata(metadata_buf);

    return m_lpRtmp->Send(metadata_buf, (int)(pbuf - metadata_buf), FLV_TAG_TYPE_META, 0);
}

char * CPushThread::WriteMetadata(char * buf)
{
    char* pbuf = buf;

    pbuf = UI08ToBytes(pbuf, AMF_DATA_TYPE_STRING);
    pbuf = AmfStringToBytes(pbuf, "@setDataFrame");

    pbuf = UI08ToBytes(pbuf, AMF_DATA_TYPE_STRING);
    pbuf = AmfStringToBytes(pbuf, "onMetaData");

	//pbuf = UI08ToBytes(pbuf, AMF_DATA_TYPE_MIXEDARRAY);
	//pbuf = UI32ToBytes(pbuf, 2);
    pbuf = UI08ToBytes(pbuf, AMF_DATA_TYPE_OBJECT);

    /*pbuf = AmfStringToBytes(pbuf, "width");
    pbuf = AmfDoubleToBytes(pbuf, 704); //width_);

    pbuf = AmfStringToBytes(pbuf, "height");
    pbuf = AmfDoubleToBytes(pbuf, 576); //height_);*/

    pbuf = AmfStringToBytes(pbuf, "framerate");
    pbuf = AmfDoubleToBytes(pbuf, 25);

    pbuf = AmfStringToBytes(pbuf, "videocodecid");
    pbuf = UI08ToBytes(pbuf, AMF_DATA_TYPE_STRING);
    pbuf = AmfStringToBytes(pbuf, "avc1");

    // 0x00 0x00 0x09
    pbuf = AmfStringToBytes(pbuf, "");
    pbuf = UI08ToBytes(pbuf, AMF_DATA_TYPE_OBJECT_END);
    
    return pbuf;
}
