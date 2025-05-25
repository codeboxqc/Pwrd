
#define WIN32_LEAN_AND_MEAN 
#include "pch.h"
#include <windows.h>
#include "resource.h"
#include <tchar.h> // Include for TCHAR and related functions
#include <commctrl.h>
#include <stdio.h> // FILE
#include <string.h> // strcpy
#include <direct.h> // mkdir
#include <shlobj.h> 
#include <shellapi.h>
#include <shlwapi.h>
#include <wchar.h> // SPLIT
#pragma comment(lib, "shlwapi.lib") 
#pragma comment(lib, "shell32.lib")

#define INITGUID

// Function prototypes
void CreateTrayIcon(HWND hWnd, HINSTANCE hInstance, int iconResourceId);
void ShowTrayIcon(void);
void HideTrayIcon(void);
void SetTrayIconIcon(HICON icon);
void SetTrayIconCaption(const TCHAR* msg);
void KillTrayIcon(void);

HICON TrayIcon = NULL;
#define WM_TRAYICON (WM_USER + 101)
NOTIFYICONDATA nid = { 0 };
BOOL g_bTrayIconVisible = FALSE;

// Function to load and create an appropriate sized icon for the system tray
HICON LoadIconForTray(HINSTANCE hInstance, int iconResourceId) {
    // System metrics for small icon size (typically used in tray)
    int cxSmIcon = GetSystemMetrics(SM_CXSMICON);
    int cySmIcon = GetSystemMetrics(SM_CYSMICON);

    // Load the icon with the appropriate size
    HICON hIcon = (HICON)LoadImage(
        hInstance,                  // Instance handle
        MAKEINTRESOURCE(iconResourceId), // Icon resource ID
        IMAGE_ICON,                 // Type: icon
        cxSmIcon,                   // Width: use system metric for small icon 
        cySmIcon,                   // Height: use system metric for small icon
        LR_DEFAULTCOLOR             // Load with default color settings
    );

    if (hIcon == NULL) {
        DWORD error = GetLastError();
        // If LoadImage fails, try LoadIcon as fallback
        hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(iconResourceId));

        // If that fails too, use a system icon
        if (hIcon == NULL) {
            hIcon = LoadIcon(NULL, IDI_APPLICATION);
        }
    }

    return hIcon;
}

void CreateTrayIcon(HWND hWnd, HINSTANCE hInstance, int iconResourceId) {
    // Initialize the structure size based on Windows version
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;

    // Load the icon with appropriate size for the tray
    TrayIcon = LoadIconForTray(hInstance, iconResourceId);
    if (TrayIcon == NULL) {
        MessageBox(hWnd, _T("Failed to load icon for system tray."), _T("Icon Error"), MB_ICONERROR);
        return;
    }

    nid.hIcon = TrayIcon;

    // Set tooltip
    _tcscpy_s(nid.szTip, _countof(nid.szTip), _T("Class"));

    // Add the icon to the system tray
    if (Shell_NotifyIcon(NIM_ADD, &nid)) {
        g_bTrayIconVisible = TRUE;
    }
    else {
        DWORD error = GetLastError();
        TCHAR errorMsg[100];
        _stprintf_s(errorMsg, _countof(errorMsg), _T("Failed to add tray icon. Error: %d"), error);
        MessageBox(hWnd, errorMsg, _T("Tray Icon Error"), MB_ICONERROR);
    }
}


void ShowTrayIcon(void)
{
    if (!Shell_NotifyIcon(NIM_ADD, &nid)) {
        DWORD error = GetLastError();
        // Handle error
    }
}

void HideTrayIcon(void)
{
    if (!Shell_NotifyIcon(NIM_DELETE, &nid)) {
        DWORD error = GetLastError();
        // Handle error
    }
}

void SetTrayIconIcon(HICON icon)
{
    nid.hIcon = icon;
    nid.uFlags = NIF_ICON;
    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void SetTrayIconCaption(const TCHAR* msg)
{
    _tcscpy_s(nid.szTip, _countof(nid.szTip), msg);
    nid.uFlags = NIF_TIP;
    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void KillTrayIcon(void)
{
    Shell_NotifyIcon(NIM_DELETE, &nid);
    if (nid.hIcon) {
        DestroyIcon(nid.hIcon);
    }
    ZeroMemory(&nid, sizeof(nid));
}