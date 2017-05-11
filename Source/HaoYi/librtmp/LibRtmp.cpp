
#include "stdafx.h"
#include "LibRtmp.h"

#include "AmfByteStream.h"
#include "..\ReadSPS.h"
#include "..\PushThread.h"
#include "..\libmp4v2\RecThread.h"

uint32_t _bs_read( bs_t *s, int i_count )
{
	static uint32_t i_mask[33] ={
		0x00,
		0x01,      0x03,      0x07,      0x0f,
		0x1f,      0x3f,      0x7f,      0xff,
		0x1ff,     0x3ff,     0x7ff,     0xfff,
		0x1fff,    0x3fff,    0x7fff,    0xffff,
		0x1ffff,   0x3ffff,   0x7ffff,   0xfffff,
		0x1fffff,  0x3fffff,  0x7fffff,  0xffffff,
		0x1ffffff, 0x3ffffff, 0x7ffffff, 0xfffffff,
		0x1fffffff,0x3fffffff,0x7fffffff,0xffffffff
	};

	int      i_shr;
	uint32_t i_result = 0;

	while( i_count > 0 )
	{
		if( s->p >= s->p_end )
		{
			break;
		}

		if( ( i_shr = s->i_left - i_count ) >= 0 )
		{
			/* more in the buffer than requested */
			i_result |= ( *s->p >> i_shr )&i_mask[i_count];
			s->i_left -= i_count;
			if( s->i_left == 0 )
			{
				s->p++;
				s->i_left = 8;
			}
			return( i_result );
		}
		else
		{
			// less in the buffer than requested
			i_result |= (*s->p&i_mask[s->i_left]) << -i_shr;
			i_count  -= s->i_left;
			s->p++;
			s->i_left = 8;
		}
	}

	return( i_result );
}

LibRtmp::LibRtmp(bool isNeedLog, bool isNeedPush, CRtmpThread * lpPullThread, CRtmpRecThread * lpRecThread)
{
	// ����rtmp���������rtmp¼�����...
	m_lpRecThread = lpRecThread;
	m_lpRtmpThread = lpPullThread;
	m_bPushStartOK = false;

    if (isNeedLog)
    {
        m_flog_ = fopen("c:\\librtmp.log", "w+");
        RTMP_LogSetLevel(RTMP_LOGDEBUG2);
        RTMP_LogSetOutput(m_flog_);
    }
    else
    {
        m_flog_ = NULL;
    }

    m_rtmp_ = RTMP_Alloc();
	RTMP_Init(m_rtmp_);

    //RTMP_SetBufferMS(m_rtmp_, 300);

    m_streming_url_ = NULL;
    m_is_need_push_ = isNeedPush;
}

LibRtmp::~LibRtmp()
{
    this->Close();
    RTMP_Free(m_rtmp_);

    if (m_streming_url_)
    {
        free(m_streming_url_);
        m_streming_url_ = NULL;
    }

    if (m_flog_) fclose(m_flog_);
}

bool LibRtmp::Open(const char* url)
{
    m_streming_url_ = (char*)calloc(strlen(url)+1, sizeof(char));
    strcpy(m_streming_url_, url);

    std::string tmp(url);
    m_stream_name_ = tmp.substr(tmp.rfind("/")+1);

    //AVal flashver = AVC("flashver");
    //AVal flashver_arg = AVC("WIN 9,0,115,0");
    AVal swfUrl = AVC("swfUrl");
    AVal swfUrl_arg = AVC("http://localhost/librtmp.swf");
    AVal pageUrl = AVC("pageUrl");
    AVal pageUrl_arg = AVC("http://localhost/librtmp.html");
    //RTMP_SetOpt(rtmp_, &flashver, &flashver_arg);
    RTMP_SetOpt(m_rtmp_, &swfUrl, &swfUrl_arg);
    RTMP_SetOpt(m_rtmp_, &pageUrl, &pageUrl_arg);

    /*if (m_is_need_record_)
    {
        AVal record = AVC("record");
        AVal record_arg = AVC("on");
        RTMP_SetOpt(m_rtmp_, &record, &record_arg);
    }*/

    int err = RTMP_SetupURL(m_rtmp_, m_streming_url_);
    if (err <= 0) return false;

	// ����ʱ���ܼ��������������Ῠ��...
	// ���ͷ���ʱ������ӣ������޷�д��...
	if( m_is_need_push_ ) {
	    RTMP_EnableWrite(m_rtmp_);
	}
    
    err = RTMP_Connect(m_rtmp_, NULL);
    if (err <= 0) return false;

    err = RTMP_ConnectStream(m_rtmp_, 0);
    if (err <= 0) return false;

    m_rtmp_->m_outChunkSize = 2*1024*1024;
    SendSetChunkSize(m_rtmp_->m_outChunkSize);

    return true;
}

void LibRtmp::Close()
{
    RTMP_Close(m_rtmp_);
}

bool LibRtmp::IsClosed()
{
	if( !RTMP_IsConnected(m_rtmp_) )
		return true;
	if( RTMP_IsTimedout(m_rtmp_) )
		return true;
	return false;
}

bool LibRtmp::Read()
{
	RTMPPacket rtmp_pakt = {0};
    RTMPPacket_Reset(&rtmp_pakt);

	int retval = RTMP_ReadPacket(m_rtmp_, &rtmp_pakt);
	if( rtmp_pakt.m_nBodySize <= 0 )
		return FALSE;
	// ����û�ж��꣬���ؼ�����ȡ...
	if( rtmp_pakt.m_nBytesRead != rtmp_pakt.m_nBodySize )
		return TRUE;
	ASSERT( rtmp_pakt.m_nBytesRead == rtmp_pakt.m_nBodySize );
	// ��ӡ����������Ϣ...
	/*if((rtmp_pakt.m_body != NULL) && (rtmp_pakt.m_nBytesRead > 0) && (rtmp_pakt.m_nBytesRead == rtmp_pakt.m_nBodySize)) {
		TRACE("HeadType = %d, Size = %d, PackType = %d, Time = %lu, absTime = %lu\r\n", 
				rtmp_pakt.m_headerType, rtmp_pakt.m_nBodySize, rtmp_pakt.m_packetType, 
				rtmp_pakt.m_nTimeStamp, rtmp_pakt.m_hasAbsTimestamp);
	}*/
	//readCallback(rtmp_pakt.m_body, rtmp_pakt.m_nBodySize, rtmp_pakt.m_packetType, rtmp_pakt.m_nTimeStamp);
	//RTMP_ClientPacket(&rtmp_pakt);

	bool is_ok = false;
	switch( rtmp_pakt.m_packetType )
	{
	case RTMP_PACKET_TYPE_AUDIO: is_ok = this->doAudio(&rtmp_pakt);  break;
	case RTMP_PACKET_TYPE_VIDEO: is_ok = this->doVideo(&rtmp_pakt);  break;
	default:					 is_ok = this->doInvoke(&rtmp_pakt); break;
	}
	RTMPPacket_Free(&rtmp_pakt);

	return is_ok;
}
//
// �����������ݰ�...
bool LibRtmp::doInvoke(RTMPPacket *packet)
{
	RTMP_ClientPacket(m_rtmp_, packet);

	return true;
}
//
// ������Ƶ֡...
bool LibRtmp::doAudio(RTMPPacket *packet)
{
	// ������Ҫ������������ݰ�...
	if((packet->m_body == NULL) || (packet->m_nBytesRead <= 0) || (packet->m_nBytesRead != packet->m_nBodySize))
		return true;
	ASSERT( packet->m_nBytesRead == packet->m_nBodySize );

	// ������Ƶ����ͷ���ᷢ�Ͷ��...
	if( packet->m_headerType == 0 ) {
		if( m_strAAC.size() <= 0 ) {
			m_strAAC.assign(packet->m_body, packet->m_nBodySize);
			this->ParseAACSequence(packet->m_body, packet->m_nBodySize);
		}
		ASSERT( m_strAAC.size() > 0 );
		return true;
	}

	// ����ⲿ¼���߳���Ч������֡����֪ͨ...
	if( m_lpRecThread != NULL ) {
		// ��ȥ���������ݿ�...
		BOOL   bWriteFlag = false;
		BYTE * lpFrame = (BYTE*)packet->m_body + 2;
		int    nSize = packet->m_nBodySize - 2;

		// �洢��Ƶ����֡...
		bWriteFlag = m_lpRecThread->WriteSample(false, lpFrame, nSize, packet->m_nTimeStamp, true);

		// ��ӡ������Ϣ...
		//TRACE("Audio = %d, Size = %d, Time = %d\r\n", packet->m_packetType, packet->m_nBodySize, packet->m_nTimeStamp);
	}
	
	// ����ⲿ�����߳���Ч������֡����֪ͨ...
	if( m_lpRtmpThread != NULL ) {
		// ���һ����Ƶ֡��֪ͨ�ϲ�...
		FMS_FRAME	theFrame;
		theFrame.typeFlvTag = FLV_TAG_TYPE_AUDIO;
		theFrame.dwSendTime = packet->m_nTimeStamp;
		theFrame.is_keyframe = true;

		// ������Ҫȥ��һ��2�ֽ����...
		theFrame.strData.assign(packet->m_body+2, packet->m_nBodySize-2);

		// ��������֡��rtmp�߳�...
		int nFrameCount = m_lpRtmpThread->PushFrame(theFrame);
		// ������һ��������֮�󣬲��������߳�...
		if( !m_bPushStartOK && nFrameCount >= 100 ) {
			m_lpRtmpThread->StartPushThread();
			m_bPushStartOK = true;
		}
	}

	return true;
}
//
// ������Ƶ֡...
bool LibRtmp::doVideo(RTMPPacket *packet)
{
	// ������Ҫ������������ݰ�...
	if((packet->m_body == NULL) || (packet->m_nBytesRead <= 0) || (packet->m_nBytesRead != packet->m_nBodySize))
		return true;
	ASSERT( packet->m_nBytesRead == packet->m_nBodySize );

	// ������Ƶ����ͷ���ᷢ�Ͷ��...
	if( packet->m_headerType == 0 ) {
		if( m_strAVC.size() <= 0 ) {
			m_strAVC.assign(packet->m_body, packet->m_nBodySize);
			this->ParseAVCSequence(packet->m_body, packet->m_nBodySize);
		}
		ASSERT( m_strAVC.size() > 0 );
		return true;
	}

	// ����ⲿ¼���߳���Ч������֡����֪ͨ...
	if( m_lpRecThread != NULL ) {
		// ��ȥ���������ݿ�...
		BOOL   bWriteFlag = false;
		BYTE * lpFrame = (BYTE*)packet->m_body + 5;
		int    nSize = packet->m_nBodySize - 5;
		BOOL   bKeyFrame = ((packet->m_body[0] == 0x17) ? true : false);

		// �洢��Ƶ����֡...
		bWriteFlag = m_lpRecThread->WriteSample(true, lpFrame, nSize, packet->m_nTimeStamp, bKeyFrame);

		// ��ӡ������Ϣ...
		//TRACE("Video = %d, Size = %d, Time = %d\r\n", packet->m_packetType, packet->m_nBodySize, packet->m_nTimeStamp);
	}

	// ����ⲿ�����߳���Ч������֡����֪ͨ...
	if( m_lpRtmpThread != NULL ) {
		// ���һ����Ƶ֡��֪ͨ�ϲ�... FLV_TAG_TYPE_VIDEO
		FMS_FRAME	theFrame;
		theFrame.typeFlvTag = FLV_TAG_TYPE_VIDEO;
		theFrame.dwSendTime = packet->m_nTimeStamp;
		theFrame.is_keyframe = ((packet->m_body[0] == 0x17) ? true : false);

		// ������Ҫȥ��һ��5�ֽ����...
		theFrame.strData.assign(packet->m_body+5, packet->m_nBodySize-5);

		// ��������֡��rtmp�߳�...
		int nFrameCount = m_lpRtmpThread->PushFrame(theFrame);
		// ������һ��������֮�󣬲��������߳�...
		if( !m_bPushStartOK && nFrameCount >= 100 ) {
			m_lpRtmpThread->StartPushThread();
			m_bPushStartOK = true;
		}
	}

	return true;
}
//
// ������Ƶ����ͷ��Ϣ...
void LibRtmp::ParseAACSequence(char * inBuf, int nSize)
{
	bs_t	s = {0};
	s.p		  = (uint8_t *)inBuf;
	s.p_start = (uint8_t *)inBuf;
	s.p_end	  = (uint8_t *)inBuf + nSize;
	s.i_left  = 8; // ����ǹ̶���,���볤��...

	// ������Ƶ�� AES ��Ϣ...
	m_strAES.assign(inBuf+2, nSize-2);

	//int soundformat = _bs_read(&s, 4);
	//int soundrate = _bs_read(&s, 2);
	//int soundsize = _bs_read(&s, 1);
	//int soundtype = _bs_read(&s, 1);
	
	int aac_flag = _bs_read(&s, 8);
	int aac_type = _bs_read(&s, 8);

	// ��ȡ��Ƶ�Ĳ�����������������...
	int object_type = _bs_read(&s, 5);
	int sample_rate_index = _bs_read(&s, 4);
	int source_channel_ = _bs_read(&s, 4);
	int	audio_sample_rate = 48000;		// ��Ƶ������
  
	// ����������ȡ������...
	switch( sample_rate_index )
	{
	case 0x03: audio_sample_rate = 48000; break;
	case 0x04: audio_sample_rate = 44100; break;
	case 0x05: audio_sample_rate = 32000; break;
	case 0x06: audio_sample_rate = 24000; break;
	case 0x07: audio_sample_rate = 22050; break;
	case 0x08: audio_sample_rate = 16000; break;
	case 0x09: audio_sample_rate = 12000; break;
	case 0x0a: audio_sample_rate = 11025; break;
	case 0x0b: audio_sample_rate =  8000; break;
	default:   audio_sample_rate = 48000; break;
	}

	// ֪ͨ¼��ģ����Ƶͷ������...
	if( m_lpRecThread != NULL ) {
		m_lpRecThread->CreateAudioTrack(audio_sample_rate, source_channel_);
	}

	// ֪ͨ�ϲ���Ƶͷ������...
	if( m_lpRtmpThread != NULL ) {
		m_lpRtmpThread->WriteAACSequenceHeader(audio_sample_rate, source_channel_);
	}
}
//
// ������Ƶ����ͷ��Ϣ...
void LibRtmp::ParseAVCSequence(char * inBuf, int nSize)
{
	bs_t	s = {0};
	s.p		  = (uint8_t *)inBuf;
	s.p_start = (uint8_t *)inBuf;
	s.p_end	  = (uint8_t *)inBuf + nSize;
	s.i_left  = 8; // ����ǹ̶���,���볤��...

	int frametype = _bs_read(&s, 4);
	int codecid   = _bs_read(&s, 4);
	int pack_type = _bs_read(&s, 8);
	int pack_time = _bs_read(&s, 24);

	int configurationVersion  = _bs_read(&s, 8);
	int AVCProfileIndication  = _bs_read(&s, 8);
	int profile_compatibility = _bs_read(&s, 8);
	int AVCLevelIndication    = _bs_read(&s, 8);
	int reserved_1			  = _bs_read(&s, 8);
	int reserved_2			  = _bs_read(&s, 8);
	int sps_size_			  = _bs_read(&s, 16);

	// ��ȡ SPS ��Ϣ...
	m_strSPS.assign((char*)s.p, sps_size_);
	s.p += sps_size_;
	s.i_left = 8;

	int nNumPPS   = _bs_read(&s, 8);
	int pps_size_ = _bs_read(&s, 16);
	
	// ��ȡ PPS ��Ϣ...
	m_strPPS.assign((char*)s.p, pps_size_);

	// ֪ͨ�ϲ���Ƶͷ������...
	if( m_lpRecThread != NULL ) {
		m_lpRecThread->CreateVideoTrack(m_strSPS, m_strPPS);
	}

	// ֪ͨ�ϲ���Ƶͷ������...
	if( m_lpRtmpThread != NULL ) {
		m_lpRtmpThread->WriteAVCSequenceHeader(m_strSPS, m_strPPS);
	}
}

bool LibRtmp::Send(const char* buf, int bufLen, int type, unsigned int timestamp)
{
    RTMPPacket rtmp_pakt;
    RTMPPacket_Reset(&rtmp_pakt);
    RTMPPacket_Alloc(&rtmp_pakt, bufLen);

    rtmp_pakt.m_packetType = type;
    rtmp_pakt.m_nBodySize = bufLen;
    rtmp_pakt.m_nTimeStamp = timestamp;
    rtmp_pakt.m_nChannel = 4;
    rtmp_pakt.m_headerType = RTMP_PACKET_SIZE_LARGE;
    rtmp_pakt.m_nInfoField2 = m_rtmp_->m_stream_id;
    memcpy(rtmp_pakt.m_body, buf, bufLen);

    int retval = RTMP_SendPacket(m_rtmp_, &rtmp_pakt, 0);
    RTMPPacket_Free(&rtmp_pakt);

    return !!retval;
}

void LibRtmp::SendSetChunkSize(unsigned int chunkSize)
{
    RTMPPacket rtmp_pakt;
    RTMPPacket_Reset(&rtmp_pakt);
    RTMPPacket_Alloc(&rtmp_pakt, 4);

    rtmp_pakt.m_packetType = 0x01;
    rtmp_pakt.m_nChannel = 0x02;    // control channel
    rtmp_pakt.m_headerType = RTMP_PACKET_SIZE_LARGE;
    rtmp_pakt.m_nInfoField2 = 0;


    rtmp_pakt.m_nBodySize = 4;
    UI32ToBytes(rtmp_pakt.m_body, chunkSize);

    RTMP_SendPacket(m_rtmp_, &rtmp_pakt, 0);
    RTMPPacket_Free(&rtmp_pakt);
}

void LibRtmp::CreateSharedObject()
{
    char data_buf[4096];
    char* pbuf = data_buf;

    pbuf = AmfStringToBytes(pbuf, m_stream_name_.c_str());

    pbuf = UI32ToBytes(pbuf, 0);    // version
    pbuf = UI32ToBytes(pbuf, 0);    // persistent
    pbuf += 4;

    pbuf = UI08ToBytes(pbuf, RTMP_SHARED_OBJECT_DATATYPE_CONNECT);

    char* pbuf_datalen = pbuf;
    pbuf += 4;

    UI32ToBytes(pbuf_datalen, (int)(pbuf - pbuf_datalen - 4));

    int buflen = (int)(pbuf - data_buf);

    LibRtmp::Send(data_buf, buflen, TAG_TYPE_SHARED_OBJECT, 0);
}

void LibRtmp::SetSharedObject(const std::string& objName, bool isSet)
{
    char data_buf[4096];
    char* pbuf = data_buf;

    pbuf = AmfStringToBytes(pbuf, m_stream_name_.c_str());

    pbuf = UI32ToBytes(pbuf, 0);    // version
    pbuf = UI32ToBytes(pbuf, 0);    // persistent
    pbuf += 4;

    pbuf = UI08ToBytes(pbuf, RTMP_SHARED_OBJECT_DATATYPE_SET_ATTRIBUTE);

    char* pbuf_datalen = pbuf;
    pbuf += 4;

    pbuf = AmfStringToBytes(pbuf, objName.c_str());
    pbuf = AmfBoolToBytes(pbuf, isSet);
    UI32ToBytes(pbuf_datalen, (int)(pbuf - pbuf_datalen - 4));

    int buflen = (int)(pbuf - data_buf);

    LibRtmp::Send(data_buf, buflen, TAG_TYPE_SHARED_OBJECT, 0);
}

void LibRtmp::SendSharedObject(const std::string& objName, int val)
{
    char data_buf[4096];
    char* pbuf = data_buf;

    pbuf = AmfStringToBytes(pbuf, m_stream_name_.c_str());

    pbuf = UI32ToBytes(pbuf, 0);    // version
    pbuf = UI32ToBytes(pbuf, 0);    // persistent
    pbuf += 4;

    pbuf = UI08ToBytes(pbuf, RTMP_SHARED_OBJECT_DATATYPE_SET_ATTRIBUTE);

    char* pbuf_datalen = pbuf;
    pbuf += 4;

    pbuf = AmfStringToBytes(pbuf, objName.c_str());
    pbuf = AmfDoubleToBytes(pbuf, val);
    UI32ToBytes(pbuf_datalen, (int)(pbuf - pbuf_datalen - 4));

    int buflen = (int)(pbuf - data_buf);

    LibRtmp::Send(data_buf, buflen, TAG_TYPE_SHARED_OBJECT, 0);
}
