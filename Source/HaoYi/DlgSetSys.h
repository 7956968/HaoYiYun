
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
	CString		m_strMainName;		// 窗口标题名称
	CString		m_strSavePath;		// 录像存放路径
	int			m_nMaxCamera;		// 最大摄像头数目
	int			m_nRecRate;			// 录像码流 => Kbps
	int			m_nLiveRate;		// 直播码流 => Kbps
	int			m_nSnapStep;		// 截图间隔 => 秒
	int			m_nRecSlice;		// 录像切片 => 秒
	BOOL		m_bAutoLinkDVR;		// 自动连接DVR设备 => 默认关闭
	BOOL		m_bAutoLinkFDFS;	// 自动连接FDFS服务器 => 默认关闭
	int			m_nWebPort;			// Web服务器端口
	CString		m_strWebAddr;		// Web服务器地址
};
