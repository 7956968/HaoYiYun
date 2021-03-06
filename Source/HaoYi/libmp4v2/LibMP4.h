
#pragma once

#include "mp4v2.h"

typedef struct RTMPFrame {
	string		m_strData;
	bool		m_bKeyFrame;
	uint32_t	m_nTimeStamp;
}RTMPFrame;

typedef	deque<RTMPFrame>	KH_DeqFrame;
class LibMP4
{
public:
	LibMP4(void);
	~LibMP4(void);
public:
	MP4Duration GetDuration();

	DWORD GetWriteSize()  { return m_dwWriteSize; }
	DWORD GetWriteRecMS() { return m_dwWriteRecMS; }
	bool IsVideoCreated() { return ((m_videoID == MP4_INVALID_TRACK_ID) ? false : true); }
	bool IsAudioCreated() { return ((m_audioID == MP4_INVALID_TRACK_ID) ? false : true); }

	bool CreateAudioTrack(const char* lpUTF8Name, int nAudioSampleRate, string & strAES);
	bool CreateVideoTrack(const char* lpUTF8Name, int nVideoTimeScale, int width, int height, string & strSPS, string & strPPS);

	bool WriteSample(bool bIsVideo, BYTE * lpFrame, int nSize, uint32_t inTimeStamp, bool bIsKeyFrame);

	bool Close();
private:
	MP4FileHandle	m_hFileHandle;		// 文件句柄
	MP4TrackId		m_videoID;			// 视频ID
	MP4TrackId		m_audioID;			// 音频ID
	RTMPFrame		m_VLastFrame;		// 上一帧视频，音频是固定的，不用记录上一帧数据
	KH_DeqFrame		m_deqAudio;			// 音频缓存数据帧
	DWORD			m_dwWriteSize;		// 写入文件长度...
	DWORD			m_dwWriteRecMS;		// 已经写入的毫秒数...
	DWORD			m_dwFirstStamp;		// 开始写盘的第一帧时间戳...
	bool			m_bFirstFrame;		// 第一帧必须写视频的关键帧,之前的音频丢掉
	int				m_nVideoTimeScale;	// 目前是固定的数值，由上层来控制并设置
};

