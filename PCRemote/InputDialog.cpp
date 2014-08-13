// InputDialog.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "PCRemote.h"
#include "InputDialog.h"
#include "afxdialogex.h"


// CInputDialog �Ի���

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


// CInputDialog ��Ϣ�������


void CInputDialog::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	if (m_strDirectory.IsEmpty())
	{
		MessageBeep(0);
		return ;		//���˳��Ի���
	}
	CDialogEx::OnOK();
}


void CInputDialog::OnBnClickedCancel()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CDialogEx::OnCancel();
}
