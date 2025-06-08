#pragma once

#include <windows.h>
#include <shellapi.h>

#pragma comment(lib,"shell32.lib")

#define WM_TRAYICON (WM_USER + 101)
#define IDM_SHOW 2221
#define IDM_HIDE 2222
//#define IDM_EXIT2 2223
#define TRAYICON_ID 2777

#define IDC_TOGGLE_STARTUP 2224 // New ID for toggle startup button

void CreateTrayIcon(HWND hWnd, HINSTANCE hInstance, int iconResourceId);
void ShowTrayIcon(void);
void HideTrayIcon(void);
void SetTrayIconIcon(HICON icon);
void SetTrayIconCaption(const TCHAR* msg); // Updated parameter type to const TCHAR*
void KillTrayIcon(void);


