// CMDBackground.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <Windows.h>

#include "Util.h"
#include <atomic>

HWND workerwHandle;

BOOL CALLBACK EnumWindowsProc(
	_In_ HWND   tophandle,
	_In_ LPARAM topparamhandle
)
{
	HWND p = FindWindowExA(tophandle, NULL, "SHELLDLL_DefView", NULL);

	if (p != NULL)
	{
		workerwHandle = FindWindowExA(NULL, tophandle, "WorkerW", NULL);
	}

	return TRUE;
}


std::atomic<bool> desktopFocused = false;
HWND cmdHandle = NULL;

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION && desktopFocused)
	{
		switch (wParam)
		{
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP:
			{

				KBDLLHOOKSTRUCT* kb = (KBDLLHOOKSTRUCT*)lParam;
				//std::cout << (char)kb->vkCode;

				/*SendMessageA(cmdHandle, WM_SETFOCUS, NULL, NULL);
				SendMessageA(cmdHandle, WM_ACTIVATE, WA_ACTIVE, NULL);
				SendMessageA(cmdHandle, WM_ACTIVATEAPP, TRUE, GetWindowThreadProcessId(cmdHandle, NULL));*/

				SetFocus(cmdHandle);
				//SendMessageA(cmdHandle, wParam, kb->vkCode, NULL);

				if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
				{
					BYTE keyboardState[256];
					if (GetKeyboardState(keyboardState) == FALSE)
						std::cout << GetLastError();

					for (int i = 0; i < 256; ++i)
					{
						keyboardState[i] = static_cast<BYTE>(GetKeyState(i));
					}

					WORD character;
					const int r = ToAscii(kb->vkCode, kb->scanCode, keyboardState, &character, 0);
					if (r != 0)
					{
						SendMessageA(cmdHandle, WM_CHAR, character, NULL);
					}
						
					//MapVirtualKey(kb->vkCode, MAPVK_VK_TO_CHAR)
					
				}
				
				//SetActiveWindow(handle);
				
			}
			break;
		}
	}
	return(CallNextHookEx(NULL, nCode, wParam, lParam));
}

int main()
{
	STARTUPINFOA info = { sizeof(info) };
	PROCESS_INFORMATION processInfo;

	char cmd[] = "CMD /C start CMD /K clear";

	if (!CreateProcessA(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo))
	{
		std::cout << "Could not start process: " << std::hex << static_cast<uint64_t>(GetLastError());
		return 1;
	}

	WaitForSingleObject(processInfo.hProcess, INFINITE);
	CloseHandle(processInfo.hProcess);
	CloseHandle(processInfo.hThread);

	
	while (cmdHandle == NULL)
	{
		cmdHandle = FindWindowA(NULL, "CMDBackground");

		if (cmdHandle == NULL)
			cmdHandle = FindWindowA(NULL, "Administrator: CMDBackground");

		Sleep(100);
	}

	std::cout << cmdHandle;

	
	

	LONG lStyle = GetWindowLong(cmdHandle, GWL_STYLE);
	lStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
	SetWindowLong(cmdHandle, GWL_STYLE, lStyle);

	

	Sleep(2000);

	

	HWND progmanHandle = FindWindowA("Progman", NULL);

	DWORD result = NULL;

	SendMessageTimeoutA(progmanHandle, 0x052C, NULL, NULL, SMTO_NORMAL, 1000, &result);

	workerwHandle = NULL;
	EnumWindows(EnumWindowsProc, NULL);

	

	SetParent(cmdHandle, workerwHandle);

	Sleep(2000);

	ShowWindow(cmdHandle, SW_MAXIMIZE);

	FreeConsole();

	const HHOOK lowLevelKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, 0, 0);

	SetTimer(NULL, 1234, 16, NULL);

	const HWND desktop = GetDesktopWindow();
	bool wasDesktop = false;

	MSG msg;

	while (GetMessage(&msg, nullptr, 0, 0))
	{

		TranslateMessage(&msg);
		DispatchMessage(&msg);

		switch (msg.message)
		{
			case WM_TIMER:
			{

				CHAR className[MAX_PATH];

				if (GetClassNameA(GetForegroundWindow(), className, MAX_PATH))
				{
					if (strcmp(className, "WorkerW") == 0)
					{
						if (!wasDesktop)
						{
							//Send keyboard data to cmd window
							desktopFocused = true;
						}

						wasDesktop = true;
					}
					else
					{
						if (wasDesktop)
						{
							//Dont send anything
							desktopFocused = false;
						}
						wasDesktop = false;
					}
				}



			}
			break;
		}
	}

	std::cout << "WTF??";
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
