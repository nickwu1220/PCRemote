// FileManagerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "PCRemote.h"
#include "FileManagerDlg.h"
#include "afxdialogex.h"
#include "FileTransferModeDlg.h"
#include "..\common\macros.h"

// CFileManagerDlg 对话框

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

	HIMAGELIST hImageList;	//加载系统图标列表
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

	//得到连接客户端的IP
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
END_MESSAGE_MAP()


// CFileManagerDlg 消息处理程序

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

	// TODO:  在此添加额外的初始化
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	RECT rect;
	GetClientRect(&rect);

	//为真彩工具条添加代码
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
	// 添加下拉按钮
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
	// 添加下拉按钮
	m_wndToolBar_Remote.AddDropDownButton(this, IDT_REMOTE_VIEW, IDR_REMOTE_VIEW);

	//显示工具栏
	m_wndToolBar_Local.MoveWindow(268, 0, rect.right - 268, 48);
	m_wndToolBar_Remote.MoveWindow(268, rect.bottom / 2 - 10, rect.right - 268, 48);

	//设置标题
	CString str;
	str.Format("\\\\%s - 文件管理", m_IPAddress);
	SetWindowText(str);

	//为列表设置ImageList
	m_list_local.SetImageList(m_pImageList_Large, LVSIL_NORMAL);
	m_list_local.SetImageList(m_pImageList_Small, LVSIL_SMALL);

	//创建带进度条的状态栏
	if (!m_wndStatusBar.Create(this) || 
		!m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return FALSE;
	}

	m_wndStatusBar.SetPaneInfo(0, m_wndStatusBar.GetItemID(0), SBPS_STRETCH, NULL);
	m_wndStatusBar.SetPaneInfo(1, m_wndStatusBar.GetItemID(1), SBPS_NORMAL, 120);
	m_wndStatusBar.SetPaneInfo(2, m_wndStatusBar.GetItemID(2), SBPS_NORMAL, 50);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0); //显示状态栏

	m_wndStatusBar.GetItemRect(1, &rect);
	m_ProgressCtrl = new CProgressCtrl;
	m_ProgressCtrl->Create(PBS_SMOOTH | WS_VISIBLE, rect, &m_wndStatusBar, 1);
	m_ProgressCtrl->SetRange(0, 100);	//设置进度条范围
	m_ProgressCtrl->SetPos(20);			//设置进度条当前位置

	//初始化本地驱动器列表并将显示本地驱动器列表
	FixedLocalDriveList();
	//初始化客户端驱动器列表并将显示客户端驱动器列表
	//FixedRmoteDriveList();

	m_bDragging = FALSE;
	m_nDragIndex = -1;
	m_nDropIndex = -1;

	CoInitialize(NULL);
	SHAutoComplete(GetDlgItem(IDC_LOCAL_PATH)->m_hWnd, SHACF_FILESYSTEM);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CFileManagerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	if(!m_wndStatusBar.m_hWnd)			//状态栏还没创建
		return ;

	// 定位状态栏
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0); //显示工具栏
	RECT	rect;
	m_wndStatusBar.GetItemRect(1, &rect);
	m_ProgressCtrl->MoveWindow(&rect);

	GetDlgItem(IDC_LIST_LOCAL)->MoveWindow(0, 36, cx, (cy - 100) / 2);
	GetDlgItem(IDC_LIST_REMOTE)->MoveWindow(0, (cy / 2) + 28, cx, (cy - 100) / 2);
	GetDlgItem(IDC_STATIC_REMOTE)->MoveWindow(20, cy / 2, 25, 20);
	GetDlgItem(IDC_REMOTE_PATH)->MoveWindow(53, (cy / 2) - 4 , 210, 12);


	GetClientRect(&rect);
	//显示工具栏
	m_wndToolBar_Local.MoveWindow(268, 0, rect.right - 268, 48);
	m_wndToolBar_Remote.MoveWindow(268, rect.bottom / 2 - 10, rect.right - 268, 48);
}

void CFileManagerDlg::FixedLocalDriveList()
{
	char szDrive[MAX_PATH] = {0};
	char *pDrive = NULL;
	m_list_local.DeleteAllItems();
	while(m_list_local.DeleteColumn(0));

	//初始化列表信息
	m_list_local.InsertColumn(0, "名称", LVCFMT_LEFT, 200);
	m_list_local.InsertColumn(1, "类型", LVCFMT_LEFT, 100);
	m_list_local.InsertColumn(2, "总大小", LVCFMT_LEFT, 100);
	m_list_local.InsertColumn(3, "可用空间", LVCFMT_LEFT, 115);

	GetLogicalDriveStrings(MAX_PATH, szDrive);
	pDrive = szDrive;

	char szFileSystem[MAX_PATH];
	unsigned __int64 HDTotalSpace = 0;
	unsigned __int64 HDFreeSpace  = 0;
	unsigned long TotalMB = 0; //总大小
	unsigned long FreeMB  = 0;  //剩余空间

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

	//重置本地当前路径
	m_Local_Path = "";
	m_Local_Directory_ComboBox.ResetContent();
}

void CFileManagerDlg::OnNMDblclkListLocal(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
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
			if (m_list_local.GetItemData(nItem) == 1)	//如有有选中，并且是目前
				directory = m_list_local.GetItemText(nItem, 0);
		} 
		else
		{
			m_Local_Directory_ComboBox.GetWindowText(m_Local_Path);
		}
	}

	//获取父目录
	if (directory == "..")
	{
		m_Local_Path = GetParentDirectory(m_Local_Path);
	}
	else if (directory != ".")		//不是刷新当前目录
	{
		m_Local_Path += directory;
		if(m_Local_Path.Right(1) != "\\")
			m_Local_Path += "\\";
	}

	//if是驱动器的根目录，返回磁盘列表
	if (m_Local_Path.GetLength() == 0)
	{
		FixedLocalDriveList();
		return ;
	}

	m_Local_Directory_ComboBox.InsertString(0, m_Local_Path);
	m_Local_Directory_ComboBox.SetCurSel(0);

	//重建列标题
	m_list_local.DeleteAllItems();
	while(m_list_local.DeleteColumn(0));

	m_list_local.InsertColumn(0, "名称", LVCFMT_LEFT, 200);
	m_list_local.InsertColumn(1, "大小", LVCFMT_LEFT, 100);
	m_list_local.InsertColumn(2, "类型", LVCFMT_LEFT, 100);
	m_list_local.InsertColumn(3, "修改日期", LVCFMT_LEFT, 115);

	int nItemIndex = 0;
	m_list_local.SetItemData(
		m_list_local.InsertItem(nItemIndex++, "..", GetIconIndex(NULL, FILE_ATTRIBUTE_NORMAL)),
		1);

	//i为0时列目录，i为1时列文件
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
		//不可能
		return ;
	}

	m_nDropIndex = -1;
}

void CFileManagerDlg::OnRemoteCopy()
{
	// TODO: 在此添加命令处理程序代码
}


void CFileManagerDlg::OnUpdateRemoteCopy(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
}


void CFileManagerDlg::OnLocalCopy()
{
	// TODO: 在此添加命令处理程序代码
	m_bIsUpload = true;

	//如果是拖拽的文件，复制到客户端去的
	if(m_nDropIndex != -1 && m_pDropList->GetItemData(m_nDropIndex))
		m_strCopyDestFolder = m_pDropList->GetItemText(m_nDropIndex, 0);	//获取文件复制到的目的文件夹

	//重置上传任务列表
	m_Remote_Upload_Job.RemoveAll();
	POSITION pos = m_list_local.GetFirstSelectedItemPosition();
	while(pos)
	{
		int nItem = m_list_local.GetNextSelectedItem(pos);
		CString file = m_Local_Path + m_list_local.GetItemText(nItem, 0);

		//如果是目录
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
		::MessageBox(m_hWnd, "文件夹为空", "警告", MB_OK|MB_ICONWARNING);
		return ;
	}

	EnableControl(FALSE);
	SendUploadJob();
}


void CFileManagerDlg::OnUpdateLocalCopy(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->Enable(m_list_local.IsWindowEnabled() 
		&& (m_Remote_Path.GetLength() || m_list_remote.GetSelectedCount())
		&& m_list_local.GetSelectedCount());
}


void CFileManagerDlg::OnBegindragListLocal(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
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

	//捕获所有的鼠标消息
	SetCapture();
	*pResult = 0;
}


void CFileManagerDlg::OnBegindragListRemote(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
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

	//捕获所有的鼠标消息
	SetCapture();
	*pResult = 0;
}


void CFileManagerDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
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
			if (m_nDropIndex != -1)//鼠标之前曾一定到过DropList里，所以m_nDropIndex得到具体的位置过，该位置项被设置高亮，之后鼠标移出DropList，需要把之前设置的高亮项取消掉	
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
	// TODO: 在此添加消息处理程序代码和/或调用默认值
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
			DropItemOnList(m_pDragList, m_pDropList);
		}
	}
	CDialogEx::OnLButtonUp(nFlags, point);
}

void CFileManagerDlg::FixedRmoteDriveList()
{
	HIMAGELIST hImageListLarge = NULL;
	HIMAGELIST hImageListSmall = NULL;
	BOOL ret = Shell_GetImageLists(&hImageListLarge, &hImageListSmall);
	ListView_SetImageList(m_list_remote.m_hWnd, hImageListLarge, LVSIL_NORMAL);
	ListView_SetImageList(m_list_remote.m_hWnd, hImageListSmall, LVSIL_SMALL);

	m_list_remote.DeleteAllItems();
	while(m_list_remote.DeleteColumn(0));

	m_list_remote.InsertColumn(0, "名称", LVCFMT_LEFT, 200);
	m_list_remote.InsertColumn(1, "类型", LVCFMT_LEFT, 100);
	m_list_remote.InsertColumn(2, "总大小", LVCFMT_LEFT, 100);
	m_list_remote.InsertColumn(3, "可用空间", LVCFMT_LEFT, 115);

	char *pDrive = NULL;
	pDrive = (char *)m_bRemoteDriveList;

	unsigned long TotalMB = 0;	//总大小
	unsigned long FreeMB  = 0;	//剩余空间
	char		  FileSystem[MAX_PATH];

	int nIconIndex = -1;

	for (int i = 0; pDrive[i] != '\0'; )
	{
		if (pDrive[i] == 'A' || pDrive[i] == 'B')
		{
			nIconIndex = 6;
		}
		else
		{
			switch(pDrive[i + 1])
			{
			case DRIVE_REMOVABLE:
				nIconIndex = 7;
				break;
			case DRIVE_FIXED:
				nIconIndex = 8;
				break;
			case DRIVE_REMOTE:
				nIconIndex = 9;
				break;
			case DRIVE_CDROM:
				nIconIndex = 11;
				break;
			default:
				nIconIndex = 8;
				break;	
			}
		}

		//磁盘名称
		CString str;
		str.Format("%c:\\", pDrive[i]);
		int nItem = m_list_remote.InsertItem(i, str, nIconIndex);
		m_list_remote.SetItemData(nItem, 1);

		//显示磁盘大小
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
		//磁盘类型为空就显示磁盘名称
		szName = (lstrlen(lpFileSystemName) == 0) ? lpTypeName : lpFileSystemName;
		m_list_remote.SetItemText(nItem, 1, szName);

		i += lstrlen(pDrive + i) + 1;
	}

	m_Remote_Path = "";
	m_Remote_Directory_ComboBox.ResetContent();
}

void CFileManagerDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CoUninitialize();
	m_pContext->m_Dialog[0] = 0;
	closesocket(m_pContext->m_Socket);

	CDialogEx::OnClose();
}

void CFileManagerDlg::OnRecviveComplete()
{
	switch(m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
	case TOKEN_FILE_LIST:	//文件列表
		FixedRemoteFileList(m_pContext->m_DeCompressionBuffer.GetBuffer(0),
							m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1);
		break;
	case TOKEN_FILE_SIZE:	// 传输文件时的第一个数据包，文件大小，及文件名
		CreateLocalRecvFile();
		break;
	case TOKEN_FILE_DATA:	//文件内容
		WriteLocalRecvFile();
		break;
	case TOKEN_TRANSFER_FINISH:		//传输完成
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

		//如果有选中的，且是目录
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

	//得到父目录
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

	//if磁盘根目录，返回磁盘列表
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

	//得到返回数据前禁用窗口
	m_list_remote.EnableWindow(FALSE);
	m_ProgressCtrl->SetPos(0);
}

void CFileManagerDlg::OnDblclkListRemote(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	if(m_list_remote.GetSelectedCount() == 0 || m_list_remote.GetItemData(m_list_remote.GetSelectionMark()) != 1)
		return ;

	GetRemoteFileList();
	*pResult = 0;
}

void CFileManagerDlg::FixedRemoteFileList(BYTE *pbBuffer, DWORD dwBufferLen)
{
	//加载系统ImageList
	SHFILEINFO sfi;
	HIMAGELIST hImageListLarge = (HIMAGELIST)SHGetFileInfo(NULL, 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_LARGEICON);
	HIMAGELIST hImageListSmall = (HIMAGELIST)SHGetFileInfo(NULL, 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
	ListView_SetImageList(m_list_remote.m_hWnd, hImageListLarge, LVSIL_NORMAL);
	ListView_SetImageList(m_list_remote.m_hWnd, hImageListSmall, LVSIL_SMALL);

	//重建标题
	m_list_remote.DeleteAllItems();
	while(m_list_remote.DeleteColumn(0));
	m_list_remote.InsertColumn(0, "名称",  LVCFMT_LEFT, 200);
	m_list_remote.InsertColumn(1, "大小", LVCFMT_LEFT, 100);
	m_list_remote.InsertColumn(2, "类型", LVCFMT_LEFT, 100);
	m_list_remote.InsertColumn(3, "修改日期", LVCFMT_LEFT, 115);

	int nItemIndex = 0;
	m_list_remote.SetItemData(
		m_list_remote.InsertItem(nItemIndex, "..", GetIconIndex(NULL, FILE_ATTRIBUTE_DIRECTORY)),
		1);

	//避免ListView闪烁，使用SetRedraw
	m_list_remote.SetRedraw(FALSE);

	if (dwBufferLen != 0)
	{
		for (int i = 0; i < 2; i++)
		{
			char *pList = (char*)pbBuffer + 1;
			for (char *pBase = pList; pList - pBase < dwBufferLen -1;)
			{
				char    *pszFileName = NULL;
				DWORD	dwFileSizeHigh = 0;		//文件高字节大小
				DWORD	dwFileSizeLow  = 0;
				bool    bIsInsert = false;
				int		nItem = 0;
				FILETIME	ftm_strReceiveLocalFileTime;

				int	nType = *pList ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
				//i为0，列目录。i为1，列文件
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

				pList += lstrlen(pszFileName) + 1;	//plist向后移动文件名长度个字节

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
	// TODO: 在此添加命令处理程序代码
	FixedLocalFileList("..");
}

//为根目录时禁用向上按钮
void CFileManagerDlg::OnUpdateLocalPrev(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->Enable(m_Local_Path.GetLength() && m_list_local.IsWindowEnabled());
}


void CFileManagerDlg::OnRemotePrev()
{
	// TODO: 在此添加命令处理程序代码
	GetRemoteFileList("..");
}

//为根目录时禁用向上按钮
void CFileManagerDlg::OnUpdateRemotePrev(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->Enable(m_Remote_Path.GetLength() && m_list_remote.IsWindowEnabled());
}


void CFileManagerDlg::OnLocalView()
{
	// TODO: 在此添加命令处理程序代码
	m_list_local.ModifyStyle(LVS_TYPEMASK, LVS_ICON);
}


void CFileManagerDlg::OnLocalList()
{
	// TODO: 在此添加命令处理程序代码
	m_list_local.ModifyStyle(LVS_TYPEMASK, LVS_LIST);
}


void CFileManagerDlg::OnLocalReport()
{
	// TODO: 在此添加命令处理程序代码
	m_list_local.ModifyStyle(LVS_TYPEMASK, LVS_REPORT);
}


void CFileManagerDlg::OnLocalBigicon()
{
	// TODO: 在此添加命令处理程序代码
	m_list_local.ModifyStyle(LVS_TYPEMASK, LVS_ICON);
}


void CFileManagerDlg::OnLocalSmallicon()
{
	// TODO: 在此添加命令处理程序代码
	m_list_local.ModifyStyle(LVS_TYPEMASK, LVS_SMALLICON);
}


void CFileManagerDlg::OnRemoteList()
{
	// TODO: 在此添加命令处理程序代码
	m_list_remote.ModifyStyle(LVS_TYPEMASK, LVS_LIST);
}


void CFileManagerDlg::OnRemoteReport()
{
	// TODO: 在此添加命令处理程序代码
	m_list_remote.ModifyStyle(LVS_TYPEMASK, LVS_REPORT);
}


void CFileManagerDlg::OnRemoteBigicon()
{
	// TODO: 在此添加命令处理程序代码
	m_list_remote.ModifyStyle(LVS_TYPEMASK, LVS_ICON);
}


void CFileManagerDlg::OnRemoteSmallicon()
{
	// TODO: 在此添加命令处理程序代码
	m_list_remote.ModifyStyle(LVS_TYPEMASK, LVS_SMALLICON);
}


void CFileManagerDlg::OnRemoteView()
{
	// TODO: 在此添加命令处理程序代码
	m_list_remote.ModifyStyle(LVS_TYPEMASK, LVS_ICON);
}


void CFileManagerDlg::OnLocalDelete()
{
	// TODO: 在此添加命令处理程序代码
	m_bIsUpload = true;
	CString str;

	if (m_list_local.GetSelectedCount() > 1)
	{
		str.Format("确定要将这 %d 项删除吗?", m_list_local.GetSelectedCount());
	} 
	else
	{
		CString file = m_list_local.GetItemText(m_list_local.GetSelectionMark(), 0);

		if(m_list_local.GetItemData(m_list_local.GetSelectionMark()) == 1)
			str.Format("确定要删除文件夹“%s”并将所有内容删除吗?", file);
		else
			str.Format("确定要把“%s”删除吗?", file);
	}

	if(::MessageBox(m_hWnd, str, "确定删除", MB_YESNO | MB_ICONQUESTION) == IDNO)
		return ;

	EnableControl(FALSE);

	POSITION pos = m_list_local.GetFirstSelectedItemPosition();
	while(pos)
	{
		int nItem = m_list_local.GetNextSelectedItem(pos);
		CString file = m_Local_Path + m_list_local.GetItemText(nItem, 0);
		//如果是目录
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

//不是根目录，并且选中项目大于0
void CFileManagerDlg::OnUpdateLocalDelete(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->Enable(m_Local_Path.GetLength() && m_list_local.GetSelectionMark() && m_list_local.IsWindowEnabled());
}


void CFileManagerDlg::OnLocalNewfolder()
{
	// TODO: 在此添加命令处理程序代码
}


void CFileManagerDlg::OnUpdateLocalNewfolder(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->Enable(m_Local_Path.GetLength() && m_list_local.IsWindowEnabled());
}


void CFileManagerDlg::OnLocalStop()
{
	// TODO: 在此添加命令处理程序代码
	m_bIsStop = TRUE;
}


void CFileManagerDlg::OnUpdateLocalStop(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->Enable(!m_list_local.IsWindowEnabled() && m_bIsUpload);
}


void CFileManagerDlg::OnRemoteDelete()
{
	// TODO: 在此添加命令处理程序代码
}


void CFileManagerDlg::OnUpdateRemoteDelete(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->Enable(m_Remote_Path.GetLength() && m_list_remote.GetSelectionMark() && m_list_remote.IsWindowEnabled());
}


void CFileManagerDlg::OnRemoteNewfolder()
{
	// TODO: 在此添加命令处理程序代码
}


void CFileManagerDlg::OnUpdateRemoteNewfolder(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->Enable(m_Remote_Path.GetLength() && m_list_remote.IsWindowEnabled());
}


void CFileManagerDlg::OnRemoteStop()
{
	// TODO: 在此添加命令处理程序代码
	m_bIsStop = TRUE;
}


void CFileManagerDlg::OnUpdateRemoteStop(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->Enable(!m_list_remote.IsWindowEnabled() && !m_bIsUpload);
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
			FixedUploadDirectory(strDirectory);		// 如果找到的是目录，则进入此目录进行递归 
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
	//如果远程列表已经选中，当作目标文件夹
	int nItem = m_list_remote.GetSelectionMark();

	//是文件夹
	if(nItem != -1 && m_list_remote.GetItemData(nItem) == 1)
	{
		strDestDirectory += m_list_remote.GetItemText(nItem, 0) + "\\";
	}

	if (!m_strCopyDestFolder.IsEmpty())
	{
		strDestDirectory += m_strCopyDestFolder + "\\";
	}

	//获取将要复制到客户文件任务列表的头一个
	m_strOperatingFile = m_Remote_Upload_Job.GetHead();

	DWORD dwSizeHigh;
	DWORD dwSizeLow;
	//// 1 字节token, 8字节大小, 文件名称, '\0'
	HANDLE hFile;
	CString fileRemote = m_strOperatingFile;	//远程文件
	//得到要保存到远程的文件路径
	fileRemote.Replace(m_Local_Path, strDestDirectory);
	m_strUploadRemoteFile = fileRemote;

	hFile = CreateFile(m_strOperatingFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if(hFile == INVALID_HANDLE_VALUE)
		return FALSE;
	dwSizeLow = GetFileSize(hFile, &dwSizeHigh);
	m_nOperatingFileLength = (dwSizeHigh * (MAXDWORD+1)) + dwSizeLow;
	CloseHandle(hFile);

	//构造数据包
	int nPacketSize = fileRemote.GetLength() + 10;
	BYTE *bPacket = (BYTE*)LocalAlloc(LPTR, nPacketSize);
	memset(bPacket, 0, nPacketSize);

	bPacket[0] = COMMAND_FILE_SIZE;
	memcpy(bPacket + 1, &dwSizeHigh, sizeof(DWORD));
	memcpy(bPacket + 5, &dwSizeLow, sizeof(DWORD));
	memcpy(bPacket + 9, fileRemote.GetBuffer(0), fileRemote.GetLength() + 1);

	m_iocpServer->Send(m_pContext, bPacket, nPacketSize);

	LocalFree(bPacket);

	//从任务列表里删除
	m_Remote_Upload_Job.RemoveHead();
	return TRUE;
}