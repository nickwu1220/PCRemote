#pragma once

#include "TrueColorToolBar.h"
#include "include/IOCPServer.h"
#include "resource.h"
#include "afxwin.h"
#include "afxcmn.h"

// CFileManagerDlg 对话框
typedef CList<CString, CString&> strList;

class CFileManagerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFileManagerDlg)

public:
	CFileManagerDlg(CWnd* pParent = NULL, CIOCPServer *pIOCPServer = NULL, ClientContext *pContext = NULL);   // 标准构造函数
	virtual ~CFileManagerDlg();

// 对话框数据
	enum { IDD = IDD_FILE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

public:
	BOOL m_bIsStop;
	CString m_strReceiveLocalFile;
	CString m_strUploadRemoteFile;
	void ShowProgress();
	void SendStop();
	int m_nTransferMode;
	CString m_strCopyDestFolder;
	void SendContinue();
	void SendException();
	void EndLocalRecvFile();
	void EndRemoteDeleteFile();
	CString m_strOperatingFile;
	__int64 m_nOperatingFileLength;	//文件size
	__int64 m_nCounter;				//计数器
	void WriteLocalRecvFile();
	void CreateLocalRecvFile();
	BOOL SendDownloadJob();
	BOOL SendUploadJob();
	BOOL SendDeleteJob();

	strList m_Remote_Download_Job;
	strList m_Remote_Upload_Job;
	strList m_Remote_Delete_Job;
	CTrueColorToolBar m_wndToolBar_Local;
	CTrueColorToolBar m_wndToolBar_Remote;
	CString m_Remote_Path;
	BYTE m_bRemoteDriveList[1024];
	CString GetParentDirectory(CString strPath);
	void OnRecviveComplete();

	CImageList *m_pImageList_Large;
	CImageList *m_pImageList_Small;

	ClientContext *m_pContext;
	CIOCPServer *m_iocpServer;
	CString m_IPAddress;

	CProgressCtrl *m_ProgressCtrl;
	HCURSOR m_hCursor;
	CString m_Local_Path;

	bool FixedUploadDirectory(LPCTSTR lpPathName);
	void FixedLocalDriveList();
	void FixedRmoteDriveList();
	void FixedLocalFileList(CString directory = "");
	void GetRemoteFileList(CString directory = "");
	void FixedRemoteFileList(BYTE *pbBuffer, DWORD dwBufferLen);

	HICON m_hIcon;
	CStatusBar m_wndStatusBar;



	CComboBox m_Local_Directory_ComboBox;
	CComboBox m_Remote_Directory_ComboBox;
	CListCtrl m_list_remote;
	CListCtrl m_list_local;
	virtual BOOL OnInitDialog();
};
