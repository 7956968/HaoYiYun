
#pragma once

class CXPButton : public CButton
{
public:
	CXPButton();
	virtual ~CXPButton();
protected:
	virtual void PreSubclassWindow();
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void DoGradientFill(CDC *pDC, CRect* rect);
	virtual void DrawInsideBorder(CDC *pDC, CRect* rect);
protected:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMouseHover(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
protected:
	BOOL  m_bOver;		// ���λ�ڰ�ť֮��ʱ��ֵΪtrue����֮Ϊflase
	BOOL  m_bTracking;	// ����갴��û���ͷ�ʱ��ֵΪtrue
	BOOL  m_bSelected;	// ��ť�������Ǹ�ֵΪtrue
	BOOL  m_bFocus;		// ��ťΪ��ǰ��������ʱ��ֵΪtrue
};
