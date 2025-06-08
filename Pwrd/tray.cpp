#define WIN32_LEAN_AND_MEAN 
#include "pch.h"
#include <windows.h>
#include "resource.h"
#include <tchar.h>
#include <commctrl.h>
#include <shellapi.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib") 
#pragma comment(lib, "shell32.lib")

#include "trayicon.h"

HICON TrayIcon = NULL;
NOTIFYICONDATA nid = { 0 };
BOOL g_bTrayIconVisible = FALSE;

HICON LoadIconForTray(HINSTANCE hInstance, int iconResourceId) {
    int cxSmIcon = GetSystemMetrics(SM_CXSMICON);
    int cySmIcon = GetSystemMetrics(SM_CYSMICON);
    HICON hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(iconResourceId), IMAGE_ICON,
        cxSmIcon, cySmIcon, LR_DEFAULTCOLOR);
    if (hIcon == NULL) {
        hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(iconResourceId));
        if (hIcon == NULL) {
            hIcon = LoadIcon(NULL, IDI_APPLICATION);
        }
    }
    return hIcon;
}

void CreateTrayIcon(HWND hWnd, HINSTANCE hInstance, int iconResourceId) {
    if (g_bTrayIconVisible) {
        return; // Prevent adding multiple tray icons
    }

    ZeroMemory(&nid, sizeof(nid));
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = TRAYICON_ID; // Use consistent ID from trayicon.h
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;

    TrayIcon = LoadIconForTray(hInstance, iconResourceId);
    if (TrayIcon == NULL) {
        MessageBox(hWnd, _T("Failed to load icon for system tray."), _T("Icon Error"), MB_ICONERROR);
        return;
    }
    nid.hIcon = TrayIcon;
    _tcscpy_s(nid.szTip, _countof(nid.szTip), _T("Pwrd"));

    if (Shell_NotifyIcon(NIM_ADD, &nid)) {
        g_bTrayIconVisible = TRUE;
    }
    else {
        DWORD error = GetLastError();
        TCHAR errorMsg[100];
        _stprintf_s(errorMsg, _countof(errorMsg), _T("Failed to add tray icon. Error: %d"), error);
        MessageBox(hWnd, errorMsg, _T("Tray Icon Error"), MB_ICONERROR);
        if (TrayIcon) {
            DestroyIcon(TrayIcon);
            TrayIcon = NULL;
        }
    }
}

void ShowTrayIcon(void) {
    if (g_bTrayIconVisible) {
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        if (!Shell_NotifyIcon(NIM_MODIFY, &nid)) {
            DWORD error = GetLastError();
            // Log error if needed
        }
    }
    else {
        Shell_NotifyIcon(NIM_ADD, &nid);
        g_bTrayIconVisible = TRUE;
    }
}

void HideTrayIcon(void) {
    if (g_bTrayIconVisible) {
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        if (Shell_NotifyIcon(NIM_DELETE, &nid)) {
            g_bTrayIconVisible = FALSE;
        }
        else {
            DWORD error = GetLastError();
            // Log error if needed
        }
    }
}

void SetTrayIconIcon(HICON icon) {
    if (icon) {
        nid.hIcon = icon;
        nid.uFlags = NIF_ICON;
        if (g_bTrayIconVisible) {
            Shell_NotifyIcon(NIM_MODIFY, &nid);
        }
    }
}

void SetTrayIconCaption(const TCHAR* msg) {
    _tcscpy_s(nid.szTip, _countof(nid.szTip), msg);
    nid.uFlags = NIF_TIP;
    if (g_bTrayIconVisible) {
        Shell_NotifyIcon(NIM_MODIFY, &nid);
    }
}

void KillTrayIcon(void) {
    if (g_bTrayIconVisible) {
        NOTIFYICONDATA tempNid = { 0 };
        tempNid.cbSize = sizeof(NOTIFYICONDATA);
        tempNid.hWnd = nid.hWnd;
        tempNid.uID = TRAYICON_ID; // Use consistent ID
        if (Shell_NotifyIcon(NIM_DELETE, &tempNid)) {
            g_bTrayIconVisible = FALSE;
        }
        else {
            DWORD error = GetLastError();
            TCHAR errorMsg[100];
            _stprintf_s(errorMsg, _countof(errorMsg), _T("Failed to remove tray icon. Error: %d"), error);
            // Optionally log error or display for debugging
            // MessageBox(NULL, errorMsg, _T("Tray Icon Removal Error"), MB_ICONERROR);
        }
    }
    if (TrayIcon) {
        DestroyIcon(TrayIcon);
        TrayIcon = NULL;
    }
    if (nid.hIcon) {
        DestroyIcon(nid.hIcon);
        nid.hIcon = NULL;
    }
    ZeroMemory(&nid, sizeof(nid));
}