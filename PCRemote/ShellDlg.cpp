// ShellDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "PCRemote.h"
#include "ShellDlg.h"
#include "afxdialogex.h"


// CShellDlg 对话框

IMPLEMENT_DYNAMIC(CShellDlg, CDialogEx)

CShellDlg::CShellDlg(CWnd* pParent = NULL, CIOCPServer* pIOCPServer = NULL, ClientContext *pContext = NULL)
	: CDialogEx(CShellDlg::IDD, pParent)
{
	m_pContext			= pContext;
	m_pIocpServer		= pIOCPServer;
	m_nCurSel			= 0;
	m_hIcon				= LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_CMDSHELL));
}

CShellDlg::~CShellDlg()
{
}

void CShellDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_edit);
}


BEGIN_MESSAGE_MAP(CShellDlg, CDialogEx)
	ON_WM_CLOSE()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CShellDlg 消息处理程序


void CShellDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_pContext->m_Dialog[0] = 0;
	closesocket(m_pContext->m_Socket);
	CDialogEx::OnClose();
}


void CShellDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	RECT rectClient;
	RECT rectEdit;
	GetClientRect(&rectClient);
	rectEdit.left = 0;
	rectEdit.top = 0;
	rectEdit.right = rectClient.right;
	rectEdit.bottom = rectClient.bottom;
	m_edit.MoveWindow(&rectEdit);
	// TODO: 在此处添加消息处理程序代码
}


BOOL CShellDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	m_nCurSel = m_edit.GetWindowTextLength();//得到当前窗口的数据大小

	//得到客户端的IP并显示到窗口的标题
	CString str;
	sockaddr_in sockAddr;
	ZeroMemory(&sockAddr, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	int nResult = getpeername(m_pContext->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);
	str.Format("\\\\%s - 远程终端", (nResult != SOCKET_ERROR) ? inet_ntoa(sockAddr.sin_addr) : "");
	SetWindowText(str);

	m_edit.SetLimitText(MAXDWORD);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}
