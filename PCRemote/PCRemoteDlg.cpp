
// PCRemoteDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "PCRemote.h"
#include "PCRemoteDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

typedef struct
{
	char *title;
	int  nWidth;
}COLUMNSTRUCT;

COLUMNSTRUCT g_Column_Online_Data[] =
{
	{"IP",				148	},
	{"区域",			150	},
	{"计算机名/备注",	160	},
	{"操作系统",		128	},
	{"CPU",				80	},
	{"摄像头",			81	},
	{"PING",			81	}
};

int g_Column_Online_Count = 7;//online列表 列的个数
int g_Column_Online_Width = 0;//online列表总宽度

COLUMNSTRUCT g_Column_Message_Data[] = 
{
	{"信息类型",		68	},
	{"时间",			100	},
	{"信息内容",	    660	}
};

int g_Column_Message_Count= 3;//Message列表 列的个数
int g_Column_Message_Width = 0;//Message列表总宽度

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CPCRemoteDlg 对话框




CPCRemoteDlg::CPCRemoteDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPCRemoteDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	iCount = 0;
}

void CPCRemoteDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ONLINE, m_CList_Online);
	DDX_Control(pDX, IDC_MESSAGE, m_CList_Message);
}

BEGIN_MESSAGE_MAP(CPCRemoteDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_ONLINE, &CPCRemoteDlg::OnNMRClickOnline)
	ON_COMMAND(IDM_ONLINE_AUDIO, &CPCRemoteDlg::OnOnlineAudio)
	ON_COMMAND(IDM_ONLINE_CMD, &CPCRemoteDlg::OnOnlineCmd)
	ON_COMMAND(IDM_ONLINE_DESKTOP, &CPCRemoteDlg::OnOnlineDesktop)
	ON_COMMAND(IDM_ONLINE_FILE, &CPCRemoteDlg::OnOnlineFile)
	ON_COMMAND(IDM_ONLINE_PROCESS, &CPCRemoteDlg::OnOnlineProcess)
	ON_COMMAND(IDM_ONLINE_REGEDIT, &CPCRemoteDlg::OnOnlineRegedit)
	ON_COMMAND(IDM_ONLINE_SERVER, &CPCRemoteDlg::OnOnlineServer)
	ON_COMMAND(IDM_ONLINE_VIDEO, &CPCRemoteDlg::OnOnlineVideo)
	ON_COMMAND(IDM_ONLINE_WINDOW, &CPCRemoteDlg::OnOnlineWindow)
	ON_COMMAND(IDM_ONLINE_DELETE, &CPCRemoteDlg::OnOnlineDelete)
	ON_COMMAND(IDM_MAIN_ABOUT, &CPCRemoteDlg::OnMainAbout)
	ON_COMMAND(IDM_MAIN_BUILD, &CPCRemoteDlg::OnMainBuild)
	ON_COMMAND(IDM_MAIN_CLOSE, &CPCRemoteDlg::OnMainClose)
	ON_COMMAND(IDM_MAIN_SET, &CPCRemoteDlg::OnMainSet)
END_MESSAGE_MAP()


// CPCRemoteDlg 消息处理程序

BOOL CPCRemoteDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_MainMemu.LoadMenu(IDR_MENU_MAIN);			//加载主菜单
	SetMenu(&m_MainMemu);

	InitList();//初始化列表
	CreateStatusBar();

	ShowMessage(true,"软件初始化成功...");

	CRect rect;
	GetWindowRect(rect);
	rect.bottom += 20;
	MoveWindow(rect);

	test();
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CPCRemoteDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CPCRemoteDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CPCRemoteDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CPCRemoteDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	double dcx = cx; //当前对话框的总宽度

	if (m_CList_Online.m_hWnd != NULL)
	{
		CRect rc;
		rc.left		= 1;
		rc.top		= 80;
		rc.right	= cx-1;
		rc.bottom	= cy-160;
		m_CList_Online.MoveWindow(rc);
		
		for (int i = 0; i < g_Column_Online_Count; i++)
		{
			double dd = g_Column_Online_Data[i].nWidth;
			dd /= g_Column_Online_Width;
			dd *= dcx;
			int lenth = dd;
			m_CList_Online.SetColumnWidth(i, lenth);
		}
	}

	if (m_CList_Message.m_hWnd != NULL)
	{
		CRect rc;
		rc.left		= 1;
		rc.top		= cy-156;
		rc.right	= cx-1;
		rc.bottom	= cy-20;
		m_CList_Message.MoveWindow(rc);

		for (int i = 0; i < g_Column_Message_Count; i++)
		{
			double dd = g_Column_Message_Data[i].nWidth;
			dd /= g_Column_Message_Width;
			dd *= dcx;
			int lenth = dd;
			m_CList_Message.SetColumnWidth(i, lenth);
		}
	}

	if (m_StatusBar.m_hWnd != NULL)
	{
		CRect rc;
		rc.top		= cy-20;
		rc.left		= 0;
		rc.right	= cx;
		rc.bottom	= cy;
		m_StatusBar.MoveWindow(rc);
		m_StatusBar.SetPaneInfo(0, m_StatusBar.GetItemID(0), SBPS_POPOUT, cx-10);
	}
}

//初始化列表
int CPCRemoteDlg::InitList(void)
{
	m_CList_Online.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_CList_Message.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	for (int i = 0; i < g_Column_Online_Count; i++)
	{
		m_CList_Online.InsertColumn(i, g_Column_Online_Data[i].title, LVCFMT_CENTER, g_Column_Online_Data[i].nWidth);
		g_Column_Online_Width += g_Column_Online_Data[i].nWidth;
	}

	for (int i = 0; i < g_Column_Message_Count; i++)
	{
		m_CList_Message.InsertColumn(i, g_Column_Message_Data[i].title, LVCFMT_CENTER, g_Column_Message_Data[i].nWidth);
		g_Column_Message_Width += g_Column_Message_Data[i].nWidth;
	}
	return 0;
}



// add to online list
void CPCRemoteDlg::AddList(CString strIP, CString strAddr, CString strPCName, CString strOS, CString strCPU, CString strVideo, CString strPing)
{
	m_CList_Online.InsertItem(0, strIP);
	m_CList_Online.SetItemText(0, ONLINELIST_ADDR, strAddr);
	m_CList_Online.SetItemText(0, ONLINELIST_COMPUTER_NAME, strPCName);
	m_CList_Online.SetItemText(0, ONLINELIST_OS, strOS);
	m_CList_Online.SetItemText(0, ONLINELIST_CPU, strCPU);
	m_CList_Online.SetItemText(0, ONLINELIST_VIDEO, strVideo);
	m_CList_Online.SetItemText(0, ONLINELIST_PING, strPing);
	ShowMessage(true, strIP+"主机上线");
}


// show msg
void CPCRemoteDlg::ShowMessage(bool bIsOK, CString strMsg)
{
	CString strIsOK, strTime;

	CTime t = CTime::GetCurrentTime();
	strTime = t.Format("%H:%M:%S");

	strIsOK = bIsOK ? "执行成功" : "执行失败";

	m_CList_Message.InsertItem(0, strIsOK);
	m_CList_Message.SetItemText(0, 1, strTime);
	m_CList_Message.SetItemText(0, 2, strMsg);

	if (strMsg.Find("上线") > 0)
	{
		iCount++;
	}else if (strMsg.Find("下线") > 0 || strMsg.Find("断开") >0)
	{
		iCount--;
	} 

	iCount = iCount < 0 ? 0 : iCount;
	CString strStatusMsg;
	strStatusMsg.Format("连接:%d", iCount);
	m_StatusBar.SetPaneText(0, strStatusMsg);


}


void CPCRemoteDlg::test(void)
{
	AddList("192.168.0.1","本机局域网","Lang","Windows7","2.2GHZ","有","123232");
	AddList("192.168.0.2","本机局域网","Lang","Windows7","2.2GHZ","有","123232");
	AddList("192.168.0.3","本机局域网","Lang","Windows7","2.2GHZ","有","123232");
}


void CPCRemoteDlg::OnNMRClickOnline(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CMenu popup;
	popup.LoadMenu(IDR_MENU_ONLINE);
	CMenu *pM = popup.GetSubMenu(0);
	CPoint p;
	GetCursorPos(&p);
	UINT count = pM->GetMenuItemCount();

	if (m_CList_Online.GetSelectedCount() == 0)
	{
		for (UINT i = 0; i < count; i++)
		{
			pM->EnableMenuItem(i, MF_BYPOSITION|MF_DISABLED|MF_GRAYED);
		}
	}
	pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);
	*pResult = 0;
}


void CPCRemoteDlg::OnOnlineAudio()
{
	// TODO: 在此添加命令处理程序代码
}


void CPCRemoteDlg::OnOnlineCmd()
{
	// TODO: 在此添加命令处理程序代码
}


void CPCRemoteDlg::OnOnlineDesktop()
{
	// TODO: 在此添加命令处理程序代码
}


void CPCRemoteDlg::OnOnlineFile()
{
	// TODO: 在此添加命令处理程序代码
}


void CPCRemoteDlg::OnOnlineProcess()
{
	// TODO: 在此添加命令处理程序代码
}


void CPCRemoteDlg::OnOnlineRegedit()
{
	// TODO: 在此添加命令处理程序代码
}


void CPCRemoteDlg::OnOnlineServer()
{
	// TODO: 在此添加命令处理程序代码
}


void CPCRemoteDlg::OnOnlineVideo()
{
	// TODO: 在此添加命令处理程序代码
}


void CPCRemoteDlg::OnOnlineWindow()
{
	// TODO: 在此添加命令处理程序代码
}


void CPCRemoteDlg::OnOnlineDelete()
{
	// TODO: 在此添加命令处理程序代码
	CString strIP;
	int nSelect = m_CList_Online.GetSelectionMark();
	strIP = m_CList_Online.GetItemText(nSelect, ONLINELIST_IP);
	m_CList_Online.DeleteItem(nSelect);
	strIP += " 断开连接";
	ShowMessage(true, strIP);
}


void CPCRemoteDlg::OnMainAbout()
{
	// TODO: 在此添加命令处理程序代码
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}


void CPCRemoteDlg::OnMainBuild()
{
	// TODO: 在此添加命令处理程序代码
}


void CPCRemoteDlg::OnMainClose()
{
	// TODO: 在此添加命令处理程序代码
	PostMessage(WM_CLOSE, 0, 0);
}


void CPCRemoteDlg::OnMainSet()
{
	// TODO: 在此添加命令处理程序代码
}

static UINT indicators[] =
{
	IDR_STATUSBAR_STRING
};



// create statusbar
void CPCRemoteDlg::CreateStatusBar(void)
{
	if (!m_StatusBar.Create(this) || 
		!m_StatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT)))
	{
		TRACE("Failed to create status bar\n");
		return ;
	}
}
