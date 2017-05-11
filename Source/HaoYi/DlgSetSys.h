
#pragma once

class CDlgSetSys : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSetSys)
public:
	CDlgSetSys(CWnd* pParent = NULL);
	virtual ~CDlgSetSys();
	enum { IDD = IDD_SET_SYS };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOK();
	DECLARE_MESSAGE_MAP()
private:
	CString		m_strMainName;		// ���ڱ�������
	CString		m_strSavePath;		// ¼����·��
	int			m_nMaxCamera;		// �������ͷ��Ŀ
	int			m_nRecRate;			// ¼������ => Kbps
	int			m_nLiveRate;		// ֱ������ => Kbps
	int			m_nSnapStep;		// ��ͼ��� => ��
	int			m_nRecSlice;		// ¼����Ƭ => ��
	BOOL		m_bAutoLinkDVR;		// �Զ�����DVR�豸 => Ĭ�Ϲر�
	BOOL		m_bAutoLinkFDFS;	// �Զ�����FDFS������ => Ĭ�Ϲر�
	int			m_nWebPort;			// Web�������˿�
	CString		m_strWebAddr;		// Web��������ַ
};