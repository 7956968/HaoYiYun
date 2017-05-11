
#pragma once

#include "SortListCtrl.h"
#include "VideoWnd.h"
#include "QRStatic.h"

class CBitItem;
class CMidView;
class CMidViewDlg : public CFormView
{
	DECLARE_DYNCREATE(CMidViewDlg)
	//DECLARE_DYNAMIC(CMidViewDlg)
public:
	CMidViewDlg(/*CWnd* pParent = NULL*/);
	virtual ~CMidViewDlg();
	enum { IDD = IDD_MIDVIEW_DLG };
protected:
	virtual void OnInitialUpdate();
	virtual void DoDataExchange(CDataExchange* pDX);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	CBitItem	**	GetBitArray()		{ return m_BitArray; }
	CFont		 *	GetVideoFont()		{ return &m_VideoFont; }
private:
	void			BuildResource();					// ������Դ...
	void			DestroyResource();					// ������Դ...
private:
	CFont			m_VideoFont;						// The font object for Video-Window...
	CMidView	*	m_lpMidView;						// ��Ƶ���ڹ�����...
	CBitItem	*	m_BitArray[CVideoWnd::kBitNum];		// ��ť�б�...
};
