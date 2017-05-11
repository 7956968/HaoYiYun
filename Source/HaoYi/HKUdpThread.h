
#pragma once

#include "OSThread.h"
#include "UDPSocket.h"

#define DEF_MCAST_ADDRV4	"239.255.255.250"	// �鲥��ַ
#define	DEF_MCAST_PORT		37020				// �鲥�˿�
#define	DEF_MCAST_STEP		30					// ���ʱ��(��)
#define DEF_MCAST_TIME		5000				// Ĭ���鲥�¼��̵߳ȴ�ʱ��(����)

class CHaoYiView;
class CHKUdpThread : public OSThread
{
public:
	CHKUdpThread(CHaoYiView * lpView);
	~CHKUdpThread();
public:
	GM_Error		InitMulticast();
private:
	virtual void	Entry();
private:
	GM_Error		ForRead();						// ��Ӧ�����ȡ�¼�
	GM_Error		SendCmdQuiry();					// ���Ͳ�ѯָ�� - �鲥����
	GM_Error		WaitAndProcMulticast();			// �ȴ��������鲥����
	GM_Error		ProcessEvent(int eventBits);	// ���������¼�
private:
	UDPSocket		m_UDPSocket;					// UDP����
	CHaoYiView	*	m_lpHaoYiView;					// ��ͼָ��
};
