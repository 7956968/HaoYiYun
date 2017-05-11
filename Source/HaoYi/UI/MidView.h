
#pragma once

#include "VideoWnd.h"

class CBitItem;
class CHaoYiView;

typedef	map<int, CVideoWnd*>	GM_MapVideo;

class CMidView : public CWnd
{
public:
	CMidView(CHaoYiView * lpParent);
	virtual ~CMidView();
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()
public:
	CFont    *		GetVideoFont() { return &m_VideoFont; }
	BOOL			IsFullScreen() { return m_bIsFullScreen; }
	BOOL			Create(UINT wStyle, const CRect & rect, CWnd * pParentWnd);
public:
	void			ReleaseFocus();						// �ͷŽ���...
	void			BuildVideoByXml();					// ͨ�����ô�������...
	void			doLeftFocus(int nCameraID);			// �����...
	void			doMoveWindow(CRect & rcArea);		// �ƶ�����...
	void			LayoutVideoWnd(int cx, int cy);		// ���д���...
	BOOL			ChangeFullScreen(BOOL bFullScreen);	// ����ȫ��ģʽ...
	CCamera     *	FindCameraByID(int nCameraID);
	CCamera		*	FindCameraBySN(string & inDeviceSN);
	GM_Error		AddNewCamera(GM_MapData & inNetData, CAMERA_TYPE inType);
	GM_Error		doCameraUDPData(GM_MapData & inNetData, CAMERA_TYPE inType);
private:
	void			FixVideoWnd();						// ��������...
	void			BuildResource();					// ������Դ...
	void			DestroyResource();					// ������Դ...
	void			DrawNotice(CDC * pDC);				// ��ӡ����...
	void			DrawBackLine(CDC * pDC);			// �����߿�...
	CCamera		*	BuildXmlCamera(GM_MapData & inXmlData);
private:
	CRect			m_rcOrigin;							// ԭʼ������
	BOOL			m_bIsFullScreen;					// ȫ����־

	CBitItem	 *	m_BitArray[CVideoWnd::kBitNum];		// ��ť�б�...
	CHaoYiView	 *	m_lpParentDlg;						// �����ڶ���...
	CFont			m_VideoFont;						// �������...

	GM_MapVideo		m_MapVideo;							// ��Ƶ���ڼ��� => VIndex => CVideoWnd
};
