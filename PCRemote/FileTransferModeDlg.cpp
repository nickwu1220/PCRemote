// FileTransferModeDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "PCRemote.h"
#include "FileTransferModeDlg.h"
#include "afxdialogex.h"


// CFileTransferModeDlg 对话框

IMPLEMENT_DYNAMIC(CFileTransferModeDlg, CDialogEx)

CFileTransferModeDlg::CFileTransferModeDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CFileTransferModeDlg::IDD, pParent)
{

}

CFileTransferModeDlg::~CFileTransferModeDlg()
{
}

void CFileTransferModeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CFileTransferModeDlg, CDialogEx)
END_MESSAGE_MAP()


// CFileTransferModeDlg 消息处理程序
