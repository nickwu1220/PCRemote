#pragma once


// CFileTransferModeDlg 对话框

class CFileTransferModeDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFileTransferModeDlg)

public:
	CFileTransferModeDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CFileTransferModeDlg();

// 对话框数据
	enum { IDD = IDD_TRANSFERMODE_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};
