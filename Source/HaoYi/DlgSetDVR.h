
#pragma once

class CDlgSetDVR : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSetDVR)
public:
	CDlgSetDVR(int nCameraID, BOOL bIsLogin, CWnd* pParent = NULL);
	virtual ~CDlgSetDVR();
	enum { IDD = IDD_SET_DVR };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOK();
	DECLARE_MESSAGE_MAP()
private:
	string		m_strDBCameraID;	// ���ݿ��е�����ͷ���
	BOOL		m_bNameChanged;		// �����Ƿ����仯
	BOOL		m_bIsLogin;			// ͨ���Ƿ��ѵ�¼
	int			m_nDVRID;			// ͨ�����
	int			m_nDVRPort;			// ��¼�˿�
	BOOL		m_bOpenOSD;			// �Ƿ���OSD
	BOOL		m_bOpenMirror;		// �Ƿ�������
	CString		m_strDVRName;		// ͨ������
	CString		m_strDVRAddr;		// ͨ����ַ
	CString		m_strLoginUser;		// ��¼�û�
	CString		m_strLoginPass;		// ��¼����

	friend class CHaoYiView;
};
