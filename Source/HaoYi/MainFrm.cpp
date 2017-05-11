
#include "stdafx.h"
#include "HaoYi.h"
#include "MainFrm.h"
#include "MidView.h"
#include "HaoYiView.h"
#include "UtilTool.h"
#include "XmlConfig.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_TIMER()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // ״̬��ָʾ��
	ID_INDICATOR_FILE,
	ID_INDICATOR_KBPS,
	ID_INDICATOR_CPU,
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

CMainFrame::CMainFrame()
{
}

CMainFrame::~CMainFrame()
{
}

HMENU CMainFrame::NewDefaultMenu()
{
	m_menuDefault.LoadMenu(IDR_MAINFRAME);
	return (m_menuDefault.Detach());
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	// ����������...
	if( !m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | 
		CBRS_ALIGN_TOP | CBRS_TOOLTIPS | CBRS_SIZE_DYNAMIC | CBRS_FLYBY | CBRS_GRIPPER) )
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	// ������Ϣ�������...
	/*if (!m_InfoBar.Create(IDD_INFOBAR,this))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}*/

	// ���Զ���ķ�ʽ���ع�����...
	this->LoadMyToolBar();

	// ���ع��������ɫλͼ�����Զ��õ�һ��������Ϊ͸��ɫ...
	m_wndToolBar.LoadTrueColorToolBar(32, IDB_BITMAP_HOT, IDB_BITMAP_HOT, IDB_BITMAP_DIS);

	// ��ʼ��Tray...RBBS_BREAK||RBBS_FIXEDSIZE|RBBS_FIXEDSIZE
	if (!m_wndReBar.Create(this, RBS_BANDBORDERS|RBS_AUTOSIZE|RBS_DBLCLKTOGGLE , WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS| WS_CLIPCHILDREN|CBRS_TOP) 
		|| !m_wndReBar.AddBar(&m_wndToolBar, RGB(224,224,224), RGB(224,224,224), NULL, RBBS_NOGRIPPER) )
	{
		TRACE0("Failed to create rebar\n");
		return -1;		// fail to create
	}
	// ����Ϣ�����뵽���ƴ�����...
	/*if (!m_wndReBar.AddBar(&m_InfoBar, RGB(224,224,224), RGB(224,224,224), NULL, RBBS_NOGRIPPER) ){
		TRACE0("Failed to create rebar\n");
		return -1;									// fail to create
	}*/
	// ����״̬��Ϣ��...
	if( !m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT)) )
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	// �޸�״̬������...
	int  nWidth = 0;
	UINT nID = 0, nStyle = 0;
	// �ϴ��ļ���...
	m_wndStatusBar.GetPaneInfo(1, nID, nStyle, nWidth);
	m_wndStatusBar.SetPaneInfo(1, nID, nStyle, 180);
	// �ϴ�������...
	m_wndStatusBar.GetPaneInfo(2, nID, nStyle, nWidth);
	m_wndStatusBar.SetPaneInfo(2, nID, nStyle, 110);
	// CPUʹ����...
	m_wndStatusBar.GetPaneInfo(3, nID, nStyle, nWidth);
	m_wndStatusBar.SetPaneInfo(3, nID, nStyle, 80);

	// ���ȣ����������ļ�...
	CXmlConfig & theConfig = CXmlConfig::GMInstance();
	if( !theConfig.GMLoadConfig() ) {
		MsgLogGM(GM_Err_Config);
		return -1;
	}
	// �ֶ�����һЩ���Բ���...
	theConfig.SetRecSlice(30);

	// ��������ʣ�����Ϊ¼���̷�...
	this->MakeGMSavePath();

	// ͳһ���������ļ�...
	theConfig.GMSaveConfig();

	// ��ʼ��CPU����...
	m_cpu.Initial();

	// ��������ʱ��...
	this->SetTimer(kStatusTimerID, 1000, NULL);

	return 0;
}
//
// ����ʱ�ӹ���...
void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
	// ����״̬��Ϣ������ʱ��...
	if( nIDEvent == kStatusTimerID ) {
		CString strText;
		CHaoYiView * lpView = (CHaoYiView*)this->GetActiveView();
		if( lpView != NULL ) {
			LPCTSTR lpszName = lpView->GetCurSendFile();
			strText.Format("�ϴ��ļ�: %s", ((lpszName != NULL) ? lpszName : "��"));
			m_wndStatusBar.SetPaneText(1, strText);
			strText.Format("�ϴ�����: %dKbps", lpView->GetCurSendKbps());
			m_wndStatusBar.SetPaneText(2, strText);
		}
		strText.Format("CPUʹ��: %d%%", m_cpu.GetCpuRate());
		m_wndStatusBar.SetPaneText(3, strText);
	}
	CFrameWnd::OnTimer(nIDEvent);
}
//
// ����¼����ͼ����·��...
GM_Error CMainFrame::MakeGMSavePath()
{
	// �鿴����·���Ƿ��Ѿ����ڣ�����·���Ƿ���Ч...
	CXmlConfig & theConfig = CXmlConfig::GMInstance();
	string strSavePath = theConfig.GetSavePath();
	// ·�����ȱ�����Ч�����һ�Ҫ��֤·���Ѿ�����...
	if( strSavePath.size() > 0 && ::PathFileExists(strSavePath.c_str()) )
		return GM_NoErr;
	// ׼����Ҫ�Ĳ�����Ϣ...
	float	 fSaveFreeGB = 0.0f;
	DWORD	 dwSize = 0;
	TCHAR	 szBuf[MAX_PATH] = {0};
	LPSTR	 lpszData = NULL;
	int		 nDiskType = 0;
	GM_Error theErr = GM_DiskErr;
	// �õ������б�...
	if( (dwSize = ::GetLogicalDriveStrings(MAX_PATH, szBuf)) <= 0 ) {
		MsgLogGM(theErr);
		return theErr;
	}
	// �����߼��ж�...
	lpszData = szBuf; ASSERT(dwSize > 0);
	while( lpszData[0] != NULL ) {
		nDiskType = ::GetDriveType(lpszData);
		do {
			// �����Ǳ��ش���...
			if( nDiskType != DRIVE_FIXED )
				break;
			// �õ���ʱ·��...
			CString strTmpDir;
			strTmpDir.Format("%s%s", lpszData, DEF_SAVE_PATH);
			// �õ���ǰ���������ʣ��ռ���ܿռ�(.2G)
			ULARGE_INTEGER	llRemain, llTotal, llFree;
			if( !GetDiskFreeSpaceEx(lpszData, &llRemain, &llTotal, &llFree) )
				break;
			// ���ֽ�ת����GB...
			BOOL	bIsFixed = ((nDiskType == DRIVE_FIXED) ? true : false);
			float   fFreeGB  = llFree.QuadPart/(1024.0f*1024.0f*1024.0f);
			float   fTotalGB = llTotal.QuadPart/(1024.0f*1024.0f*1024.0f);
			// ʣ����������ģ���������...
			if( fFreeGB >= fSaveFreeGB ) {
				strSavePath = strTmpDir;
				fSaveFreeGB = fFreeGB;
				break;
			}
			ASSERT( fFreeGB < fSaveFreeGB );
		}while( false );
		// �ƶ������б���������ָ��...
		lpszData += (strlen(lpszData) + 1);
	}
	// �Ի�ȡ��·������֮...
	if( strSavePath.size() <= 0 ) {
		strSavePath = "C:\\GMSave";
	}
	// �������·���������ļ�����...
	ASSERT( strSavePath.size() > 0 );
	theConfig.SetSavePath(strSavePath);
	//theConfig.GMSaveConfig();
	// ��������·��Ŀ¼...
	if( !::PathFileExists(strSavePath.c_str()) ) {
		CUtilTool::CreateDir(strSavePath.c_str());
	}
	return GM_NoErr;
}
//
// �޸Ĺ�������Ϣ...
void CMainFrame::LoadMyToolBar()
{
	// ���ù�����λͼ�߿�...
	SIZE sizeButton, sizeImage;
	sizeButton.cx = 55;	sizeButton.cy = 52;
	sizeImage.cx = 32;	sizeImage.cy = 32;
	m_wndToolBar.SetSizes(sizeButton, sizeImage);

	//ȡ��ToolBar��CToolBarCtrl
    CToolBarCtrl& oBarCtrl = m_wndToolBar.GetToolBarCtrl();

	// ��������...
	const int nItemCount = 6;
	UINT uCtrlID[nItemCount] = {
		ID_VIEW_FULL, ID_LOGIN_DVR, ID_LOGOUT_DVR,
		ID_DVR_SET,   ID_SYS_SET,   ID_APP_ABOUT, 
	};
	LPCTSTR lpszCtrl[nItemCount] = {
		"ȫ��ģʽ", "��¼ͨ��", "ע��ͨ��", 
		"ͨ������", "ϵͳ����", "��ʾ�汾", 
	};

	// �ָ�����ť...
	TBBUTTON sepButton = {0};
    sepButton.fsState = TBSTATE_ENABLED;
    sepButton.fsStyle = TBSTYLE_SEP;

	// ���䰴ť...
	TBBUTTON * pTBButtons = new TBBUTTON[nItemCount];
	ASSERT(pTBButtons != NULL);
    for( int nIndex = 0; nIndex < nItemCount; ++nIndex)
    {
		// ���ð�ť����...
        pTBButtons[nIndex].iString = oBarCtrl.AddStrings(lpszCtrl[nIndex]);	// �����ַ���
        pTBButtons[nIndex].iBitmap = nIndex;								// ����Bitmap����
        pTBButtons[nIndex].idCommand = uCtrlID[nIndex];						// ControlID
        pTBButtons[nIndex].fsState = TBSTATE_ENABLED;
        pTBButtons[nIndex].fsStyle = TBSTYLE_BUTTON;
        pTBButtons[nIndex].dwData = 0;
		// ���Ӱ�ť...
		VERIFY( oBarCtrl.AddButtons(1, &pTBButtons[nIndex]) );
		// ���ӿո�...
		if( nIndex == 0 || nIndex == 2 || nIndex == 4 || nIndex == 5 ) {
			VERIFY( oBarCtrl.AddButtons(1, &sepButton) );
		}
    }
	// �ͷŰ�ť...
    delete[] pTBButtons;
	pTBButtons = NULL;
    oBarCtrl.AutoSize();
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// ȥ�� EX_CLIENT ���Լ���GDI...
	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	return TRUE;
}

// CMainFrame ���
#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	return CFrameWnd::OnCreateClient(lpcs, pContext);
}

void CMainFrame::OnClose()
{
	// �˳�֮ǰ��ѯ��...
	if( ::MessageBox(this->m_hWnd, "ȷʵҪ�˳�ϵͳ��", "ȷ��", MB_OKCANCEL | MB_ICONWARNING) == IDCANCEL )
		return;
	// �ͷſ����Դ...
	CFrameWnd::OnClose();
}