#pragma once


// CFileTransferModeDlg �Ի���

class CFileTransferModeDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFileTransferModeDlg)

public:
	CFileTransferModeDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CFileTransferModeDlg();

// �Ի�������
	enum { IDD = IDD_TRANSFERMODE_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
};
