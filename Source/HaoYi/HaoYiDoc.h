
#pragma once

class CHaoYiDoc : public CDocument
{
protected: // �������л�����
	CHaoYiDoc();
	DECLARE_DYNCREATE(CHaoYiDoc)
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
public:
	virtual ~CHaoYiDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	DECLARE_MESSAGE_MAP()
};
