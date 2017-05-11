
#pragma once

#include "rtmp.h"
#include "log.h"

class CRtmpRecThread;
class CRtmpThread;
class LibRtmp
{
public:
    LibRtmp(bool isNeedLog, bool isNeedPush, CRtmpThread * lpPullThread, CRtmpRecThread * lpRecThread);

    ~LibRtmp();

    bool Open(const char* url);
	bool IsClosed();

    void Close();

	bool Read();
    bool Send(const char* buf, int bufLen, int type, unsigned int timestamp);

    void SendSetChunkSize(unsigned int chunkSize);

    void CreateSharedObject();

    void SetSharedObject(const std::string& objName, bool isSet);

    void SendSharedObject(const std::string& objName, int val);
private:
	bool doAudio(RTMPPacket *packet);
	bool doVideo(RTMPPacket *packet);
	bool doInvoke(RTMPPacket *packet);
	void ParseAACSequence(char * inBuf, int nSize);
	void ParseAVCSequence(char * inBuf, int nSize);
private:
    RTMP	*	m_rtmp_;				// rtmp����
    char	*	m_streming_url_;		// ����ַ
    FILE	*	m_flog_;				// ��־���
    bool		m_is_need_push_;		// �Ƿ�����
    string		m_stream_name_;			// ������

	string		m_strAAC;				// for audio
	string		m_strAVC;				// for video
	string		m_strAES;				// for audio
	string		m_strSPS;				// SPS...
	string		m_strPPS;				// PPS...

	BOOL			  m_bPushStartOK;	// �Ƿ��Ѿ������߳�...
	CRtmpThread	   *  m_lpRtmpThread;	// �ⲿ�̶߳���...
	CRtmpRecThread *  m_lpRecThread;	// �ⲿ¼���̶߳���...
};
