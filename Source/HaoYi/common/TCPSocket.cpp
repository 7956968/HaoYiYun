
#include "stdafx.h"
#include "UtilTool.h"
#include "TCPSocket.h"

TCPSocket::TCPSocket()
{
}

TCPSocket::~TCPSocket()
{
}

/*GM_Error TCPSocket::Set(SOCKET inSocket, sockaddr_in* remoteAddr)
{
	m_RemoteAddr = *remoteAddr;
	m_hSocket = inSocket;

	if( inSocket != INVALID_SOCKET ) {
		// make sure to find out what IP address this connection is actually occuring on. That
		// way, we can report correct information to clients asking what the connection's IP is
		int len = sizeof(m_LocalAddr);
		int err = ::getsockname(m_hSocket, (sockaddr*)&m_LocalAddr, &len);
		ASSERT(err == 0);
		this->Linger( LINGER_TIME );	//�õײ���δ����������ǰ��Ҫ�������	
	}
	return GM_NoErr;
}*/
//
// ֻ�ʺ� IPV4 �����ӷ�ʽ...
GM_Error TCPSocket::ConnectV4(UInt32 inRemoteAddr, UInt16 inRemotePort)
{
	m_bIsIPV6 = false;
	::memset(&m_RemoteAddrV4, 0, sizeof(m_RemoteAddrV4));
	m_RemoteAddrV4.sin_family = AF_INET;				/* host byte order */
	m_RemoteAddrV4.sin_port = htons(inRemotePort);	/* short, network byte order */
	m_RemoteAddrV4.sin_addr.s_addr = htonl(inRemoteAddr);

	int err = ::connect(m_hSocket, (sockaddr *)&m_RemoteAddrV4, sizeof(m_RemoteAddrV4));

	if( err == -1 && GetLastError() != WSAEWOULDBLOCK ) {
		m_RemoteAddrV4.sin_port = 0;
		m_RemoteAddrV4.sin_addr.s_addr = 0;
		return (GM_Error)GetLastError();
	}
	return GM_NoErr;
}
//
// ͨ�õ��ʺ� IPV4/IPV6 �����ӷ�ʽ...
GM_Error TCPSocket::Connect(LPCTSTR lpszRemoteAddr, int inRemotePort)
{
	// �����ж������������Ч��...
	GM_Error theErr = GM_BadInputValue;
	if( lpszRemoteAddr == NULL || strlen(lpszRemoteAddr) <= 0 || inRemotePort <= 0 || inRemotePort >= 65535 ) {
		MsgLogGM(theErr);
		return theErr;
	}
	// �ò��ҷ�ʽ��ȡ��Ч��ַ...
	ADDRINFO   theHints = {0};
	ADDRINFO * lpInfoVal = NULL;
	theHints.ai_family = PF_UNSPEC;
	theHints.ai_socktype = SOCK_STREAM;
	theHints.ai_flags = AI_NUMERICHOST | AI_PASSIVE;

	// ����˿ڵ�ַ...
	TCHAR szPort[64] = {0};
	sprintf(szPort, "%lu", inRemotePort);

	// ͨ���ַ�����ȡ��Ч��ַ...
	theErr = getaddrinfo(lpszRemoteAddr, szPort, &theHints, &lpInfoVal);
	if( theErr != GM_NoErr ) {
		MsgLogGM(theErr);
		return theErr;
	}
	theErr = GM_BadInputValue;
	if( lpInfoVal == NULL || lpInfoVal->ai_addrlen <= 0 ) {
		MsgLogGM(theErr);
		return theErr;
	}
	// ����IPV4�ĵ�ַ��Ϣ...
	if( !m_bIsIPV6 ) {
		memcpy(&m_RemoteAddrV4, lpInfoVal->ai_addr, sizeof(m_RemoteAddrV4));
	}
	// ֱ�����ӷ�����...
	ASSERT( lpInfoVal != NULL && lpInfoVal->ai_addrlen > 0 );
	return this->Connect(lpInfoVal->ai_addr, lpInfoVal->ai_addrlen);
}
//
// ͨ�õ��ʺ� IPV4/IPV6 �����ӷ�ʽ...
GM_Error TCPSocket::Connect(SOCKADDR * lpConAddr, int nAddrLen)
{
	GM_Error theErr = GM_BadInputParam;
	if( lpConAddr == NULL || nAddrLen <= 0 )
		return theErr;
	ASSERT( lpConAddr != NULL && nAddrLen > 0 );

	int err = ::connect(m_hSocket, lpConAddr, nAddrLen);

	if( err == -1 && GetLastError() != WSAEWOULDBLOCK ) {
		return (GM_Error)GetLastError();
	}
	return GM_NoErr;
}
