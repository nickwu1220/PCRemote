#pragma once
#include "afxcmn.h"
#include "include/IOCPServer.h"

// CSystemDlg �Ի���

class CSystemDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSystemDlg)

public:
	CSystemDlg(CWnd* pParent = NULL, CIOCPServer* pIOCPServer = NULL, ClientContext *pContext = NULL);   // ��׼���캯��
	virtual ~CSystemDlg();

// �Ի�������
	enum { IDD = IDD_SYSTEM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	CTabCtrl m_tab;
	CListCtrl m_list_process;
	CListCtrl m_list_windows;

private:
	HICON m_hIcon;
	ClientContext* m_pContext;
	CIOCPServer* m_iocpServer;
public:
	void AdjustList(void);
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
	void ShowSelectWindow(void);
	void GetProcessList(void);
	virtual BOOL OnInitDialog();
	void ShowProcessList(void);
};
