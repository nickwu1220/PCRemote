#pragma once
#include "afxcmn.h"
#include "include/IOCPServer.h"

// CSystemDlg 对话框

class CSystemDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSystemDlg)

public:
	CSystemDlg(CWnd* pParent = NULL, CIOCPServer* pIOCPServer = NULL, ClientContext *pContext = NULL);   // 标准构造函数
	virtual ~CSystemDlg();

// 对话框数据
	enum { IDD = IDD_SYSTEM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CTabCtrl m_tab;
	CListCtrl m_list_process;
	CListCtrl m_list_windows;

private:
	HICON m_hIcon;
	ClientContext* m_pContext;
	CIOCPServer* m_iocpServer;
	int m_ProcessListWidth;			//初始时Process列表，各列宽度总和
	int m_WindowsListWidth;			//初始时Winodws列表，各列宽度总和

	BOOL m_bAsc;			//列表数据升序排列
	int m_nSortCol;
	static int CALLBACK MyCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort); //排序比较函数
public:
	void AdjustList();
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
	void ShowSelectWindow(void);
	void GetProcessList(void);
	virtual BOOL OnInitDialog();
	void ShowProcessList(void);
	afx_msg void OnKillprocess();
	afx_msg void OnRefreshpslist();
	afx_msg void OnNMRClickListProcess(NMHDR *pNMHDR, LRESULT *pResult);
	void OnReceiveComplete(void);
	afx_msg void OnLvnColumnclickListProcess(NMHDR *pNMHDR, LRESULT *pResult);
	void GetWindowsList(void);
	void ShowWindowsList(void);
};
