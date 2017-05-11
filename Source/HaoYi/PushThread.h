
#pragma once

#include "OSMutex.h"
#include "OSThread.h"
#include "mp4v2.h"
#include "myRTSPClient.h"

typedef struct
{
    int			typeFlvTag;			// FLV_TAG_TYPE_AUDIO or FLV_TAG_TYPE_VIDEO
    bool		is_keyframe;		// �Ƿ��ǹؼ�֡
	uint32_t	dwSendTime;			// ����ʱ��(����)
	string		strData;			// ֡����
}FMS_FRAME;

// ���尴ʱ������Ŀ����ظ���֡����(����Ƶ��϶���)...
typedef	multimap<uint32_t, FMS_FRAME>	KH_MapFrame;

class LibRtmp;
class CCamera;
class CPushThread;
class CRtspThread : public OSThread
{
public:
	CRtspThread();
	~CRtspThread();
public:
	void			StartPushThread();
	void			PushFrame(FMS_FRAME & inFrame);
	BOOL			InitRtsp(CPushThread * inPushTread, CString & strRtspUrl);
	string	 &		GetAVCHeader() { return m_strAVCHeader; }
	string	 &		GetAACHeader() { return m_strAACHeader; }
	bool			IsFinished()   { return m_bFinished; }
	void			WriteAVCSequenceHeader(string & inSPS, string & inPPS);
	void			WriteAACSequenceHeader(int inAudioRate, int inAudioChannel);
	void			ResetEventLoop() { m_rtspEventLoopWatchVariable = 1; }
private:
	virtual void	Entry();
private:
	OSMutex			m_Mutex;
	string			m_strRtspUrl;
	CPushThread *	m_lpPushThread;
	
	bool			m_bFinished;			// �������Ƿ������

	int				m_audio_rate_index;
	int				m_audio_channel_num;

	string			m_strSPS;				// SPS
	string			m_strPPS;				// PPS
	string			m_strAACHeader;			// AAC
	string			m_strAVCHeader;			// AVC

	TaskScheduler * m_scheduler_;			// rtsp��Ҫ���������
	UsageEnvironment * m_env_;				// rtsp��Ҫ�Ļ���
	ourRTSPClient * m_rtspClient_;			// rtsp����
	char m_rtspEventLoopWatchVariable;		// rtsp�˳���־ => �¼�ѭ����־���������Ϳ��Կ��������߳�...
};

class CRtmpThread : public OSThread
{
public:
	CRtmpThread();
	~CRtmpThread();
public:
	void			StartPushThread();
	int				PushFrame(FMS_FRAME & inFrame);
	BOOL			InitRtmp(CPushThread * inPushTread);
	string	 &		GetAVCHeader() { return m_strAVCHeader; }
	string	 &		GetAACHeader() { return m_strAACHeader; }
	bool			IsFinished()   { return m_bFinished; }
	void			WriteAVCSequenceHeader(string & inSPS, string & inPPS);
	void			WriteAACSequenceHeader(int inAudioRate, int inAudioChannel);
private:
	virtual void	Entry();
private:
	OSMutex			m_Mutex;
	string			m_strRtmpUrl;
	CPushThread *	m_lpPushThread;
	
	bool			m_bFinished;			// �������Ƿ������

	int				m_audio_rate_index;
	int				m_audio_channel_num;

	string			m_strSPS;				// SPS
	string			m_strPPS;				// PPS
	string			m_strAACHeader;			// AAC
	string			m_strAVCHeader;			// AVC

	LibRtmp	  *		m_lpRtmp;				// rtmp��������...
};

class CPushThread : public OSThread
{
public:
	CPushThread(HWND hWndParent);
	~CPushThread();
public:
	BOOL			InitThread(CCamera * lpCamera, CString & strRtspUrl, string & strRtmpUrl);
	int				PushFrame(FMS_FRAME & inFrame);
	int				GetSendKbps() { return m_nSendKbps; }
	void			doErrNotify();
private:
	virtual	void	Entry();

	BOOL			OpenRtmpUrl();
	int				SendOneDataPacket();
    BOOL			SendVideoDataPacket(FMS_FRAME & inFrame);
    BOOL			SendAudioDataPacket(FMS_FRAME & inFrame);
	BOOL			SendAACSequenceHeaderPacket();
	BOOL			SendAVCSequenceHeaderPacket();
	char		*	WriteMetadata(char * buf);
	int				SendMetadataPacket();

	bool			IsDataFinished();
	BOOL			IsFrameTimeout();
private:
	OSMutex			m_Mutex;
	bool			m_bFileMode;
	string			m_strRtmpUrl;
	uint32_t		m_nSendKbps;		// ��������
	uint32_t		m_nCurSendByte;		// ��ǰ�ѷ����ֽ���
	DWORD			m_dwTimeOutMS;		// ��ʱ��ʱ��
	HWND			m_hWndParent;		// �����ڶ���
	bool			m_bDeleteFlag;		// ��ɾ����־
	
	CCamera		*	m_lpCamera;			// ����ͷ����...
	CRtmpThread	*	m_lpRtmpThread;		// rtmp�߳�...
	CRtspThread	*	m_lpRtspThread;		// rtsp�߳�...

	LibRtmp		*	m_lpRtmp;			// rtmp���Ͷ���...
	KH_MapFrame		m_MapFrame;			// ��ʱ�������֡���ݶ���

	friend class CCamera;
};
