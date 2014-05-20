// ShellDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "PCRemote.h"
#include "ShellDlg.h"
#include "afxdialogex.h"


// CShellDlg �Ի���

IMPLEMENT_DYNAMIC(CShellDlg, CDialogEx)

CShellDlg::CShellDlg(CWnd* pParent = NULL, CIOCPServer* pIOCPServer = NULL, ClientContext *pContext = NULL)
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

	m_edit.SetLimitText(MAXDWORD);
	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}
