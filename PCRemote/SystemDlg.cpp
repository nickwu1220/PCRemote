// SystemDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "PCRemote.h"
#include "SystemDlg.h"
#include "afxdialogex.h"
#include "..\common\macros.h"

// CSystemDlg �Ի���

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
	ON_COMMAND(IDM_KILLPROCESS, &CSystemDlg::OnKillprocess)
	ON_COMMAND(IDM_REFRESHPSLIST, &CSystemDlg::OnRefreshpslist)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_PROCESS, &CSystemDlg::OnNMRClickListProcess)
END_MESSAGE_MAP()


// CSystemDlg ��Ϣ�������


void CSystemDlg::AdjustList(void)
{
	if (m_list_process.m_hWnd==NULL)
	{
		return;
	}
	if (m_list_windows.m_hWnd==NULL)
	{
		return;
	}

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
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	m_pContext->m_Dialog[0] = 0;
	closesocket(m_pContext->m_Socket);
	CDialogEx::OnClose();
}


void CSystemDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	//AdjustList();
	// TODO: �ڴ˴������Ϣ����������
}


void CSystemDlg::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	//�õ��ͻ��˵�IP����ʾ�����ڵı���
	CString str;
	sockaddr_in sockAddr;
	ZeroMemory(&sockAddr, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	int nResult = getpeername(m_pContext->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);
	str.Format("\\\\%s - ϵͳ����", (nResult != SOCKET_ERROR) ? inet_ntoa(sockAddr.sin_addr) : "");
	SetWindowText(str);

	m_tab.InsertItem(0, "���̹���");
	m_tab.InsertItem(1, "���ڹ���");

	m_list_process.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT);  //��ʼ�����̵��б�
	m_list_process.InsertColumn(0, "ӳ������", LVCFMT_LEFT, 100);
	m_list_process.InsertColumn(1, "PID", LVCFMT_LEFT, 50);
	m_list_process.InsertColumn(2, "����·��", LVCFMT_LEFT, 400);

	m_list_windows.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT);  //��ʼ�� ���ڹ�����б�
	m_list_windows.InsertColumn(0, "PID", LVCFMT_LEFT, 50);
	m_list_windows.InsertColumn(1, "��������", LVCFMT_LEFT, 300);

	//AdjustList();			//�����б��С
	ShowProcessList();
	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}


void CSystemDlg::ShowProcessList(void)
{
	char *pBuffer = (char*)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	DWORD dwOffset = 0;
	char *szExeFile;
	char *szProcessFullName;
	CString str;

	m_list_process.DeleteAllItems();

	//������������ÿһ���ַ��������������ݽṹ�� Id+������+0+������+0
	int i = 0;
	for (; dwOffset < m_pContext->m_DeCompressionBuffer.GetBufferLen()-1; i++)
	{
		DWORD *pdwID      = (DWORD*)(pBuffer + dwOffset);
		szExeFile         = pBuffer + dwOffset + sizeof(DWORD);
		szProcessFullName = pBuffer + lstrlen(szExeFile) + 1;

		m_list_process.InsertItem(i, szExeFile);
		str.Format("%5u", *pdwID);
		m_list_process.SetItemText(i, 1, str);
		m_list_process.SetItemText(i, 2, szProcessFullName);

		dwOffset += sizeof(DWORD) + lstrlen(szExeFile) + lstrlen(szProcessFullName) + 2;
	}

}


void CSystemDlg::OnKillprocess()
{
	// TODO: �ڴ���������������
	CListCtrl *pListCtrl = NULL;
	if(m_list_process.IsWindowEnabled())
		pListCtrl = &m_list_process;
	else if(m_list_windows.IsWindowEnabled())
		pListCtrl = &m_list_windows;
	else
		return ;

	LPBYTE pBuffer = (LPBYTE)LocalAlloc(LPTR, 1 + (pListCtrl->GetSelectedColumn() * sizeof(DWORD)));
	pBuffer[0] = COMMAND_KILLPROCESS;

	//��ʾ������Ϣ
	char *lpTips = "����: ��ֹ���̻ᵼ�²�ϣ�������Ľ����\n"
		"�������ݶ�ʧ��ϵͳ���ȶ����ڱ���ֹǰ��\n"
		"���̽�û�л��ᱣ����״̬�����ݡ�";
	CString str;
	if (pListCtrl->GetSelectedCount() > 1)
	{
		str.Format("%sȷʵ\n����ֹ��%d�������?", lpTips, pListCtrl->GetSelectedCount());	
	}
	else
	{
		str.Format("%sȷʵ\n����ֹ���������?", lpTips);
	}
	if (::MessageBox(m_hWnd, str, "���̽�������", MB_YESNO|MB_ICONQUESTION) == IDNO)
		return;

	DWORD dwOffset = 1;
	POSITION pos = pListCtrl->GetFirstSelectedItemPosition();
	while(pos)
	{
		int nItem = pListCtrl->GetNextSelectedItem(pos);
		CString str = pListCtrl->GetItemText(nItem, 1);
		DWORD dwProcessID = atoi(str);
		memcpy(pBuffer  + dwOffset, &dwProcessID, sizeof(DWORD));
		dwOffset += sizeof(DWORD);
	}

	//�������ݵ��ͻ��ˣ��ͻ��˲���COMMAND_KILLPROCESS�������ͷ
	m_iocpServer->Send(m_pContext, pBuffer, LocalSize(pBuffer));
	LocalFree(pBuffer);
}




void CSystemDlg::OnRefreshpslist()
{
	// TODO: �ڴ���������������
	if(m_list_process.IsWindowVisible())
		GetProcessList();
}


void CSystemDlg::OnNMRClickListProcess(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CMenu popup;
	popup.LoadMenu(IDR_PSLIST);
	CMenu *pMenuSub = popup.GetSubMenu(0);
	CPoint p;
	GetCursorPos(&p);

	if (m_list_process.GetSelectedCount() == 0)
	{
		pMenuSub->EnableMenuItem(0, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
	}
	pMenuSub->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);
	*pResult = 0;
}


void CSystemDlg::OnReceiveComplete(void)
{
	switch(m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
	case TOKEN_PSLIST:
		ShowProcessList();
		break;
	default:
		break;
	}
}
