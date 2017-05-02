// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "stdio.h"

typedef int(*__stdcall MOVECARDS)(HWND);
typedef int(*__fastcall MainWndProc)(LPWSTR, int, HWND, int, int, int);
typedef int(*__stdcall CleanUp)(HWND);

HWND FindMyTopMostWindow()
{
	DWORD dwProcID = GetCurrentProcessId();
	HWND hWnd = GetTopWindow(GetDesktopWindow());
	while (hWnd)
	{
		DWORD dwWndProcID = 0;
		GetWindowThreadProcessId(hWnd, &dwWndProcID);
		if (dwWndProcID == dwProcID)
			return hWnd;
		hWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
	}
	return NULL;
}

//Make the invalid move state Not in this Game
void problem_1() {
	DWORD s = (DWORD)GetModuleHandle(NULL);
	DWORD offset = (DWORD)0x00010C04;
	DWORD a = offset + s;
	DWORD old;
	char* ptr = (char*)a;
	VirtualProtect(ptr, 0x31, 0x40, &old);
	char msg[] = "Not in this game.";
	for (int x = 0; x < sizeof(msg) / sizeof(msg[0]); x++)
		ptr[x * 2] = msg[x];
}

//Set total wins to 1000
void problem_2() {
	// store key in freecell mem
	HKEY* _hkey = (HKEY*)0x10079A4;
	// pointer to registry path
	WCHAR* _pszRegPath = (WCHAR*)0x1001230;
	// pointer to pszWon variable
	WCHAR* _pszWon = (WCHAR*)0x10012A8;
	// create registry key - store in _hkey
	RegCreateKeyW((HKEY)2147483649, _pszRegPath, _hkey);
	// set registry at _pszWon to data
	DWORD data = 1000;
	RegSetValueExW(*_hkey, _pszWon, 0, 3, (LPBYTE)&data, 4);
	// close registry key
	RegCloseKey(*_hkey);
}

void problem_5() {
	HWND hWnd = FindMyTopMostWindow();
	//To make sure that moves continue to happen
	*(unsigned int *)0x01007864 = 1;
	//To make sure that the cards are 0 so that you can win.
	//If cards are not zero, that means that there is still cards left in play
	*(unsigned int *)0x01007800 = 0;

	//move the rest of the cards to win
	MOVECARDS mcFn = (MOVECARDS)0x01004FC7;
	mcFn(hWnd);
}

HHOOK hkb;
KBDLLHOOKSTRUCT kbdStruct;

HACCEL newFreeMenu;
HACCEL origFreeMenu;

//Returns the hwnd to the top most window.
//src: http://stackoverflow.com/questions/1125564/getting-the-hwnd-for-my-own-application-in-c


void MessageHandler(HWND hWnd, LPMSG uMsg, WPARAM wParam, LPARAM lParam) {

	if (!TranslateAccelerator(hWnd, newFreeMenu, uMsg))
	{
		TranslateMessage(uMsg);
		DispatchMessage(uMsg);
	}
}

LRESULT CALLBACK HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
	char buf[200];

	LPMSG lMsg = (LPMSG)lParam;
	HWND hWnd;
	hWnd = *(HWND *)0x01008374;

	if (TranslateAccelerator(hWnd, newFreeMenu, lMsg) == 0)
	{
		DWORD err = GetLastError();

		TranslateMessage(lMsg);
		DispatchMessage(lMsg);
	}
	else {
		//If you push the f2 combination, problem 5 should be
		//solved. Make sure you start a new game first
		if (lMsg->lParam == 0x3C0001) {
			problem_5();

			return 0;
		}
	}

	// call the next hook in the hook chain. This is nessecary or your hook chain will break and the hook stops
	return CallNextHookEx(hkb, nCode, wParam, lParam);
}

//Install the hook to the accelerator table. And get the messages accordingly
BOOL InstallHook()
{

	char buf[200];

	HWND hWnd = FindMyTopMostWindow();

	DWORD dwProcID = GetCurrentProcessId();

	DWORD dwThreadID = GetWindowThreadProcessId(hWnd, &dwProcID);


	HWND pgmHwnd;
	pgmHwnd = *(HWND *)0x01008374;

	hkb = SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)HookCallback, NULL, dwThreadID);


	MSG msg;
	BOOL bRet;

	while (bRet = GetMessage(&msg, NULL, 0, 0))
	{
		Sleep(100);
	}

	if (hkb == NULL)
		return FALSE;
	else
		return TRUE;
}

//Destroy the old accelerator table and create new one.
void newAccelerators(HMODULE self) {

	origFreeMenu = LoadAccelerators(NULL, L"FreeMenu");
	if (origFreeMenu != NULL) {
		DestroyAcceleratorTable(origFreeMenu);
	}
	newFreeMenu = LoadAccelerators(self, L"FreeMenu");

}






BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH: {
		//ALL problems WERE solved with this code

		//Problem 1
		//Make an invalid move say not in this game
		problem_1();

		//Problem 2
		//Set the total wins to 1000
		problem_2();

		//Problem 3
		//This just makes the cheating variable equal to 2 so that you can win on next turn
		*(unsigned int *)0x1007130 = 2;

		//Problem 4
		//Create the new accelerator table with the rc file
		newAccelerators(hModule);
		//Hook the accelerator table to freecell
		InstallHook();

		//Problem 5 is outlined in hook callback else statement
		//above

	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		return TRUE;
	}
}

