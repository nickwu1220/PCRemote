// InputDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "PCRemote.h"
#include "InputDialog.h"
#include "afxdialogex.h"


// CInputDialog 对话框

IMPLEMENT_DYNAMIC(CInputDialog, CDialogEx)

CInputDialog::CInputDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(CInputDialog::IDD, pParent)
	, m_strDirectory(_T(""))
{

}

CInputDialog::~CInputDialog()
{
}

void CInputDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_strDirectory);
}


BEGIN_MESSAGE_MAP(CInputDialog, CDialogEx)
	ON_BN_CLICKED(IDOK, &CInputDialog::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CInputDialog::OnBnClickedCancel)
END_MESSAGE_MAP()


// CInputDialog 消息处理程序


void CInputDialog::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if (m_strDirectory.IsEmpty())
	{
		MessageBeep(0);
		return ;		//不退出对话框
	}
	CDialogEx::OnOK();
}


void CInputDialog::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}
