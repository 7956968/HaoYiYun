
#include "stdafx.h"
// SHARED_HANDLERS ������ʵ��Ԥ��������ͼ������ɸѡ�������
// ATL ��Ŀ�н��ж��壬�����������Ŀ�����ĵ����롣
#ifndef SHARED_HANDLERS
#include "HaoYi.h"
#endif

#include "HaoYiDoc.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CHaoYiDoc, CDocument)

BEGIN_MESSAGE_MAP(CHaoYiDoc, CDocument)
END_MESSAGE_MAP()

CHaoYiDoc::CHaoYiDoc()
{
}

CHaoYiDoc::~CHaoYiDoc()
{
}

BOOL CHaoYiDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: �ڴ�������³�ʼ������
	// (SDI �ĵ������ø��ĵ�)

	return TRUE;
}

void CHaoYiDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: �ڴ���Ӵ洢����
	}
	else
	{
		// TODO: �ڴ���Ӽ��ش���
	}
}

#ifdef _DEBUG
void CHaoYiDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CHaoYiDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG
