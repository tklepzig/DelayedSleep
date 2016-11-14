#include <tchar.h>
#include <Windows.h>
#include <WindowsX.h>
#include <iostream>
#include <fstream>
#include <PowrProf.h>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "PowrProf.lib")

using namespace std;

HINSTANCE globalHInstance;

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_POWERBROADCAST:
	{
		if (wParam == PBT_POWERSETTINGCHANGE)
		{
			POWERBROADCAST_SETTING* pbs = (POWERBROADCAST_SETTING*)lParam;

			if (pbs->PowerSetting == GUID_LIDSWITCH_STATE_CHANGE && pbs->DataLength == sizeof(int))
			{
				if (*pbs->Data == 0)
				{
					//sleep after 1 minute
					SetTimer(hWnd, 42, 60 * 1000, (TIMERPROC)NULL);
				}
				else if (*pbs->Data == 1)
				{
					KillTimer(hWnd, 42);
				}
			}
		}

		break;
	}
	case WM_TIMER:
	{
		switch (wParam)
		{
		case 42:
			SetSuspendState(false, true, true);
			return 0;
		}
	}
	case WM_HOTKEY:
	{
		if (wParam == 1)
		{
			MessageBox(0, TEXT("DelayedSleep halted."), TEXT("DelayedSleep"), MB_OK);
			PostQuitMessage(0);
		}
	}
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		break;
	}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nShowCmd)
{
#pragma region allow only one instance

	HANDLE hMutex = CreateMutex(0, FALSE, TEXT("{5BAF5FB5-AAB4-4479-B2E1-CC38B629BF99}"));

	if (hMutex && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		hMutex = 0;
		return 0;
	}

	ReleaseMutex(hMutex);

#pragma endregion	

	globalHInstance = hInstance;

	WNDCLASSEX wndClass = { 0 };
	wndClass.cbSize = sizeof(wndClass);
	wndClass.lpfnWndProc = WndProc;
	wndClass.hInstance = hInstance;
	wndClass.lpszClassName = TEXT("DelayedSleep");
	RegisterClassEx(&wndClass);

	HWND hWnd = CreateWindowEx(0, TEXT("DelayedSleep"), TEXT("DelayedSleep"), 0, 20, 20, 200, 200, 0, 0, hInstance, 0);
	HPOWERNOTIFY hPowerNotify = RegisterPowerSettingNotification(hWnd, &GUID_LIDSWITCH_STATE_CHANGE, 0);

	RegisterHotKey(hWnd, 1, MOD_WIN | MOD_CONTROL, VK_F12);

	MSG msg;
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	PlaySound(0, 0, 0);

	UnregisterPowerSettingNotification(hPowerNotify);

	return msg.wParam;
}