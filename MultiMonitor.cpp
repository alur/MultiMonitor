//--------------------------------------------------------------------------------------
// File: MultiMonitor.cpp
//--------------------------------------------------------------------------------------
#define WIN32_LEAN_AND_MEAN
#include "lsapi.h"
#include <stdlib.h>
#include <strsafe.h>
#include <windows.h>


//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
static int g_lsMessages[] = { LM_GETREVID, LM_REFRESH, 0 };
static int g_iNumMonitors, g_iMonNum, g_iPrimaryMonitor;
static char g_OnMonitorConnected[MAX_LINE_LENGTH];
static char g_OnMonitorDisconnected[MAX_LINE_LENGTH];
static char g_OnPrimaryMonitorChanged[MAX_LINE_LENGTH];
static HWND g_hwndMessageHandler;


//--------------------------------------------------------------------------------------
// Function declarations
//--------------------------------------------------------------------------------------
extern "C" __declspec( dllexport ) int initModuleEx(HWND hwndParent, HINSTANCE hDllInstance, LPCSTR szPath);
extern "C" __declspec( dllexport ) void quitModule(HINSTANCE hDllInstance);
bool CreateMessageHandler(HINSTANCE hInst);
void SetEvars();
void ReadSettings();
void ClearOldEvars();
LRESULT WINAPI MessageHandlerProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK SetMonitorEvars(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);


//--------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------
const char g_rcsRevision[]	= "1.3";
const char g_szAppName[]	= "MultiMonitor";
const char g_szMsgHandler[]	= "LSMultiMonitorManager";
const char g_szAuthor[]		= "Alurcard2";


//--------------------------------------------------------------------------------------
// DLL entry point
//--------------------------------------------------------------------------------------
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID /* lpReserved */)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
		DisableThreadLibraryCalls((HINSTANCE)hModule);
    }

	return TRUE;
}

//--------------------------------------------------------------------------------------
// Called when the module is loaded
//--------------------------------------------------------------------------------------
int initModuleEx(HWND /* hwndParent */, HINSTANCE hDllInstance, LPCSTR /* szPath */ )
{
    if (!CreateMessageHandler(hDllInstance))
    {
		return 1;
    }

	SetEvars();
	ReadSettings();
	return 0;
}

//--------------------------------------------------------------------------------------
// Called when the module is about to be unloaded
//--------------------------------------------------------------------------------------
void quitModule(HINSTANCE hDllInstance)
{
	if (g_hwndMessageHandler)
	{
		SendMessage(GetLitestepWnd(), LM_UNREGISTERMESSAGE, (WPARAM)g_hwndMessageHandler, (LPARAM)g_lsMessages);
		DestroyWindow(g_hwndMessageHandler);
	}
	UnregisterClass(g_szMsgHandler, hDllInstance);
}

//--------------------------------------------------------------------------------------
// Called once for every monitor by LSEnumDisplayMonitors
//--------------------------------------------------------------------------------------
BOOL CALLBACK SetMonitorEvars(HMONITOR hMonitor, HDC /* hdcMonitor */, LPRECT /* lprcMonitor */, LPARAM /* dwData */)
{
	char szNum[32], szEvar[MAX_RCCOMMAND];
	int iMonNum;
	
	// Get info about the monitor
	MONITORINFO mi;
	mi.cbSize = sizeof(MONITORINFO);
	LSGetMonitorInfo(hMonitor, &mi);

	// Check if this monitor is the primary monitor
	if ((mi.dwFlags & MONITORINFOF_PRIMARY) == MONITORINFOF_PRIMARY)
	{
		iMonNum = 1;
		g_iPrimaryMonitor = g_iMonNum;
	}
	else
	{
		iMonNum = ++g_iMonNum + 1;
	}
	
	_itoa_s(mi.rcMonitor.top, szNum, sizeof(szNum), 10);
	sprintf_s(szEvar, MAX_RCCOMMAND, "Monitor%dTop", iMonNum);
	LSSetVariable(szEvar, szNum);
	
	_itoa_s(mi.rcMonitor.left, szNum, sizeof(szNum), 10);
	sprintf_s(szEvar, MAX_RCCOMMAND, "Monitor%dLeft", iMonNum);
	LSSetVariable(szEvar, szNum);
	
	_itoa_s(mi.rcMonitor.bottom, szNum, sizeof(szNum), 10);
	sprintf_s(szEvar, MAX_RCCOMMAND, "Monitor%dBottom", iMonNum);
	LSSetVariable(szEvar, szNum);
	
	_itoa_s(mi.rcMonitor.right, szNum, sizeof(szNum), 10);
	sprintf_s(szEvar, MAX_RCCOMMAND, "Monitor%dRight", iMonNum);
	LSSetVariable(szEvar, szNum);

	_itoa_s(mi.rcMonitor.right - mi.rcMonitor.left, szNum, sizeof(szNum), 10);
	sprintf_s(szEvar, MAX_RCCOMMAND, "Monitor%dResolutionX", iMonNum);
	LSSetVariable(szEvar, szNum);
	
	_itoa_s(mi.rcMonitor.bottom - mi.rcMonitor.top, szNum, sizeof(szNum), 10);
	sprintf_s(szEvar, MAX_RCCOMMAND, "Monitor%dResolutionY", iMonNum);
	LSSetVariable(szEvar, szNum);
	return true;
}

//--------------------------------------------------------------------------------------
// Clears all old evars
//--------------------------------------------------------------------------------------
void ClearOldEvars()
{
	char szEvar[MAX_RCCOMMAND];
	for (int i = 1; i <= g_iMonNum; i++)
	{
		sprintf_s(szEvar, MAX_RCCOMMAND, "Monitor%dTop", i);
		LSSetVariable(szEvar, "");
		sprintf_s(szEvar, MAX_RCCOMMAND, "Monitor%dLeft", i);
		LSSetVariable(szEvar, "");
		sprintf_s(szEvar, MAX_RCCOMMAND, "Monitor%dBottom", i);
		LSSetVariable(szEvar, "");
		sprintf_s(szEvar, MAX_RCCOMMAND, "Monitor%dRight", i);
		LSSetVariable(szEvar, "");
		sprintf_s(szEvar, MAX_RCCOMMAND, "Monitor%dResolutionX", i);
		LSSetVariable(szEvar, "");
		sprintf_s(szEvar, MAX_RCCOMMAND, "Monitor%dResolutionY", i);
		LSSetVariable(szEvar, "");
	}
}

void ReadSettings()
{
	GetRCLine("OnMonitorConnected", g_OnMonitorConnected, MAX_LINE_LENGTH, "");
	GetRCLine("OnMonitorDisconnected", g_OnMonitorDisconnected, MAX_LINE_LENGTH, "");
	GetRCLine("OnPrimaryMonitorChanged", g_OnPrimaryMonitorChanged, MAX_LINE_LENGTH, "");
}

void SetEvars()
{
	char szNum[10];
	g_iNumMonitors = LSGetSystemMetrics(SM_CMONITORS);
	_itoa_s(g_iNumMonitors, szNum, 10, 10);
	LSSetVariable("NumberOfMonitors", szNum);
	_itoa_s(LSGetSystemMetrics(SM_XVIRTUALSCREEN), szNum, 10, 10);
	LSSetVariable("VirtualScreenX", szNum);
	_itoa_s(LSGetSystemMetrics(SM_YVIRTUALSCREEN), szNum, 10, 10);
	LSSetVariable("VirtualScreenY", szNum);
	_itoa_s(LSGetSystemMetrics(SM_CXVIRTUALSCREEN), szNum, 10, 10);
	LSSetVariable("VirtualScreenResolutionX", szNum);
	_itoa_s(LSGetSystemMetrics(SM_CYVIRTUALSCREEN), szNum, 10, 10);
	LSSetVariable("VirtualScreenResolutionY", szNum);
	g_iMonNum = 0;
	LSEnumDisplayMonitors(NULL, NULL, SetMonitorEvars, 0);
}

bool CreateMessageHandler(HINSTANCE hInst)
{
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = MessageHandlerProc;
	wc.hInstance = hInst;
	wc.lpszClassName = g_szMsgHandler;
	wc.hIconSm = 0;

	if (!RegisterClassEx(&wc))
		return false;

	g_hwndMessageHandler = CreateWindowEx(WS_EX_TOOLWINDOW, g_szMsgHandler, 0, WS_POPUP, 0, 0, 0, 0, 0, 0, hInst, 0);

	if (!g_hwndMessageHandler)
		return false;

	SendMessage(GetLitestepWnd(), LM_REGISTERMESSAGE, (WPARAM)g_hwndMessageHandler, (LPARAM) g_lsMessages);

	return true;
}

LRESULT WINAPI MessageHandlerProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg) {
	case LM_REFRESH:
        {
		    ReadSettings();
	    }
	    return 0;

	case LM_GETREVID:
        {
		    size_t uLength;
		    StringCchPrintf((char*)lParam, 64, "%s: %s", g_szAppName, g_rcsRevision);
            if (SUCCEEDED(StringCchLength((char*) lParam, 64, &uLength)))
            {
			    return uLength;
            }
		    lParam = 0;
		}
		return 0;

	case WM_DISPLAYCHANGE:
        {
			int oldmonitorcount = g_iNumMonitors;
			int oldprimarymonitor = g_iPrimaryMonitor;
			ClearOldEvars();
			SetEvars();
			if (oldmonitorcount < g_iNumMonitors)
				LSExecute(nullptr, g_OnMonitorConnected, SW_SHOWNORMAL);
			else if (oldmonitorcount > g_iNumMonitors)
				LSExecute(nullptr, g_OnMonitorDisconnected, SW_SHOWNORMAL);
			else if (g_iPrimaryMonitor != oldprimarymonitor)
				LSExecute(nullptr, g_OnPrimaryMonitorChanged, SW_SHOWNORMAL);
		}
		return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}