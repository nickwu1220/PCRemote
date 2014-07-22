// TestDll.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Windows.h>

int _tmain(int argc, _TCHAR* argv[])
{
	char strHost[] = "127.0.0.1";
	//char strHost[] = "192.168.1.62";
	int nPort = 3000;

	HMODULE hServerDll = LoadLibrary("..\\..\\bin\\server\\server.dll");
	//HMODULE hServerDll = LoadLibrary("server.dll");
	typedef void (_cdecl *TestRunFun)(char* strHost, int nPort);
	TestRunFun pTestRun = (TestRunFun)GetProcAddress(hServerDll, "TestRun");

	if (pTestRun)
	{
		pTestRun(strHost, nPort);
	}
	return 0;
}

