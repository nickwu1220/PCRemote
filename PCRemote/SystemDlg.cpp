// SystemDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "PCRemote.h"
#include "SystemDlg.h"
#include "afxdialogex.h"


// CSystemDlg 对话框

IMPLEMENT_DYNAMIC(CSystemDlg, CDialogEx)

CSystemDlg::CSystemDlg(CWnd* pParent, CIOCPServer* pIOCPServer, ClientContext *pContext)
	: CDialogEx(CSystemDlg::IDD, pParent)
{
	m_iocpServer = pIOCPServer;
	m_pContext = pContext;
	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_SYSTEM));
}

CSystemDlg::~CSystemDlg()
{
}

void CSystemDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB1, m_tab);
	DDX_Control(pDX, IDC_LIST_PROCESS, m_list_process);
	DDX_Control(pDX, IDC_LIST_WINDOWS, m_list_windows);
}


BEGIN_MESSAGE_MAP(CSystemDlg, CDialogEx)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CSystemDlg::OnTcnSelchangeTab1)
END_MESSAGE_MAP()


// CSystemDlg 消息处理程序


void CSystemDlg::AdjustList(void)
{
	RECT rectClient;
	RECT rectlist;
	GetClientRect(&rectClient);
	rectlist.left = 0;
	rectlist.top = 29;
	rectlist.right = rectClient.right;
	rectClient.bottom = rectClient.bottom;

	m_list_process.MoveWindow(&rectlist);
	m_list_windows.MoveWindow(&rectlist);
}


void CSystemDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_pContext->m_Dialog[0] = 0;
	closesocket(m_pContext->m_Socket);
	CDialogEx::OnClose();
}


void CSystemDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	AdjustList();
	// TODO: 在此处添加消息处理程序代码
}


void CSystemDlg::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	ShowSelectWindow();
	*pResult = 0;
}


void CSystemDlg::ShowSelectWindow(void)
{
	switch (m_tab.GetCurSel())
	{
	case 0:
		m_list_windows.ShowWindow(SW_HIDE);
		m_list_process.ShowWindow(SW_SHOW);
		if(m_list_process.GetItemCount() == 0)
			GetProcessList();
		break;
	case 1:
		m_list_windows.ShowWindow(SW_SHOW);
		m_list_process.ShowWindow(SW_HIDE);
		if(m_list_windows.GetItemCount() == 0)
			//GetWindowsList();
		break;
	default:
		break;
	}
}


void CSystemDlg::GetProcessList(void)
{
	BYTE bToken = COMMAND_PSLIST;
	m_iocpServer->Send(m_pContext, &bToken, sizeof(BYTE));
}


BOOL CSystemDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	//得到客户端的IP并显示到窗口的标题
	CString str;
	sockaddr_in sockAddr;
	ZeroMemory(&sockAddr, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	int nResult = getpeername(m_pContext->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);
	str.Format("\\\\%s - 系统管理", (nResult != SOCKET_ERROR) ? inet_ntoa(sockAddr.sin_addr) : "");
	SetWindowText(str);

	m_tab.InsertItem(0, "进程管理");
	m_tab.InsertItem(1, "窗口管理");

	m_list_process.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT);  //初始化进程的列表
	m_list_process.InsertColumn(0, "映像名称", LVCFMT_LEFT, 100);
	m_list_process.InsertColumn(1, "PID", LVCFMT_LEFT, 50);
	m_list_process.InsertColumn(2, "程序路径", LVCFMT_LEFT, 400);

	m_list_windows.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT);  //初始化 窗口管理的列表
	m_list_windows.InsertColumn(0, "PID", LVCFMT_LEFT, 50);
	m_list_windows.InsertColumn(1, "窗口名称", LVCFMT_LEFT, 300);

	AdjustList();			//调整列表大小
	ShowProcessList();
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CSystemDlg::ShowProcessList(void)
{
	char *pBuffer = (char*)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	m_list_process.DeleteAllItems();

	//遍历发送来的每一个字符别忘了他的数据结构啊 Id+进程名+0+完整名+0


}
