// CheatAssistantDll.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "Lua.h"

#define MY_MESSAGE_ID 10024

Lua lua;
WNDPROC g_orgProc;
bool g_luaStatus = false;
int g_threadStatus = 0;

HANDLE hMutex;

// 设置lua入口开关 
bool setSwitchStatus(bool status)
{
	lua_getglobal(lua.L, "g_switch");
	bool m_oldSwitch = lua_toboolean(lua.L, -1);//原开关状态
	lua_pop(lua.L,1);
	if (m_oldSwitch != status)//不相等就修改
	{
		lua_pushboolean(lua.L, status);
		lua_setglobal(lua.L, "g_switch");
	}
	return m_oldSwitch;
}

static void luaThread()
{
	while (true)
	{
		WaitForSingleObject(hMutex, INFINITE);
		if (g_luaStatus) {
			lua_getglobal(lua.L, "main");
			lua.check(lua_pcall(lua.L, 0, 0, 0));
		}
		ReleaseMutex(hMutex);
	}
}

static  void OnKeyDown(HWND hWnd, UINT wParam, BOOL bCtl, INT lParamL, UINT lParamH)
{
	switch (wParam)
	{
	case VK_HOME:
		WaitForSingleObject(hMutex, INFINITE);
		g_luaStatus = lua.doFile("C:\\lua\\main.lua");//载入lua入口文件
		ReleaseMutex(hMutex);
		break;
	default:
		//bool m_oldSwitch = setSwitchStatus(false);
		WaitForSingleObject(hMutex, INFINITE);
		if (g_luaStatus)
		{
			lua_getglobal(lua.L, "onKeyDown");
			lua_pushinteger(lua.L, (lua_Integer)hWnd);
			lua_pushinteger(lua.L, (lua_Integer)wParam);
			lua_pushinteger(lua.L, (lua_Integer)bCtl);
			lua_pushinteger(lua.L, (lua_Integer)lParamL);
			lua_pushinteger(lua.L, (lua_Integer)lParamH);
			lua.check(lua_pcall(lua.L, 5, 0, 0));          //用保护模式调用lua函数，入参个数为5、出参个数为0、无自定义错误处理
		}
		//setSwitchStatus(m_oldSwitch);
		ReleaseMutex(hMutex);
		break;
	}
}

static  void OnSysKeyDown(HWND hWnd, UINT nChar, BOOL bCtl, UINT nRepCnt, UINT nFlags)
{
	//bool m_oldSwitch = setSwitchStatus(false);
	WaitForSingleObject(hMutex, INFINITE);
	if (g_luaStatus)
	{
		lua_getglobal(lua.L, "onSysKeyDown");
		lua_pushinteger(lua.L, (lua_Integer)hWnd);
		lua_pushinteger(lua.L, (lua_Integer)nChar);
		lua_pushinteger(lua.L, (lua_Integer)bCtl);
		lua_pushinteger(lua.L, (lua_Integer)nRepCnt);
		lua_pushinteger(lua.L, (lua_Integer)nFlags);
		lua.check(lua_pcall(lua.L, 5, 0, 0));          //用保护模式调用lua函数，入参个数为5、出参个数为0、无自定义错误处理
	}
	//setSwitchStatus(m_oldSwitch);
	ReleaseMutex(hMutex);
}

static LRESULT CALLBACK CallWndProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) 
{
	switch (uMsg)
	{
		// 设置热键
		HANDLE_MSG(hWnd, WM_KEYDOWN, OnKeyDown);
		HANDLE_MSG(hWnd, WM_SYSKEYDOWN, OnSysKeyDown);
		break;
	default:
		if (uMsg == MY_MESSAGE_ID)
		{
			//bool m_oldSwitch = setSwitchStatus(false);
			WaitForSingleObject(hMutex, INFINITE);
			if (g_luaStatus)
			{
				lua_getglobal(lua.L, "CallWndProc");
				lua_pushinteger(lua.L, (lua_Integer)hWnd);
				lua_pushinteger(lua.L, (lua_Integer)uMsg);
				lua_pushinteger(lua.L, (lua_Integer)wParam);
				lua_pushinteger(lua.L, (lua_Integer)lParam);
				lua.check(lua_pcall(lua.L, 4, 0, 0));          //用保护模式调用lua函数，入参个数为4、出参个数为0、无自定义错误处理
			}
			//setSwitchStatus(m_oldSwitch);
			ReleaseMutex(hMutex);
		}
		break;
	}
	
	return CallWindowProc(g_orgProc, hWnd, uMsg, wParam, lParam);
}

static void initializeThread()
{
	while (g_hWnd == NULL)
	{
		//OutputDebugString(L"Lua initializeThread");
		//g_hWnd = FindWindow(L"YodaoMainWndClass", L"网易有道词典");
		g_hWnd = FindWindow(L"地下城与勇士", L"地下城与勇士");
		Sleep(1000);
	}
	g_orgProc = (WNDPROC)SetWindowLongA(g_hWnd, GWL_WNDPROC, (LONG_PTR)CallWndProc);
	// 把窗口句柄传给lua
	lua_pushinteger(lua.L, (lua_Integer)g_hWnd);
	lua_setglobal(lua.L,"g_hWnd");
	// 创建lua线程
	HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)luaThread, NULL, 0, 0);
	// 创建互斥量
	wchar_t buffer[0x100];
	swprintf_s(buffer,L"mochv354y0XrTzy8%d",GetCurrentThreadId());
	hMutex = CreateMutex(NULL, FALSE, buffer);
	CloseHandle(hThread);
}

extern "C" __declspec(dllexport)void entryPoint(
	M_OPEN_VIDPID m_Open_VidPid,
	M_RELEASEALLKEY m_ReleaseAllKey,
	M_KEYSTATE2 m_KeyState2,
	M_KEYDOWN2 m_KeyDown2,
	M_KEYUP2 m_KeyUp2,
	M_LEFTCLICK m_LeftClick,
	M_MOVETO2 m_MoveTo2,
	M_MOVETO3 m_MoveTo3,
	M_GETCURMOUSEPOS2 m_GetCurrMousePos2,
	M_CLOSE m_Close,
	M_RESOLUTIONUSED m_ResolutionUsed
) 
{
	LoadLibrary(_T("C:\\Windows\\System32\\dsrole.dll"));
	M_Open_VidPid = m_Open_VidPid;
	M_ReleaseAllKey = m_ReleaseAllKey;
	M_KeyState2 = m_KeyState2;
	M_KeyDown2 = m_KeyDown2;
	M_KeyUp2 = m_KeyUp2;
	M_LeftClick = m_LeftClick;
	M_MoveTo2 = m_MoveTo2;
	M_MoveTo3 = m_MoveTo3;
	M_GetCurrMousePos2 = m_GetCurrMousePos2;
	M_Close = m_Close;
	M_ResolutionUsed = m_ResolutionUsed;

	g_msdkHandle = M_Open_VidPid(0xc310, 0xc007);
	if (g_msdkHandle == INVALID_HANDLE_VALUE) {
		MessageBox(NULL, L"", VMProtectDecryptStringW(L"端口打开失败，请确认您的设备已经插上电脑"), MB_OK);
	}
	else {
		M_ResolutionUsed(g_msdkHandle, 1920, 1080);
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)initializeThread, NULL, 0, 0);
	}
}

//extern "C" __declspec(dllexport)void entryPoint2(
//	
//)
//{
//	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)initializeThread, NULL, 0, 0);
//}