#define WIN32_LEAN_AND_MEAN
#include "pch.h"
#include "framework.h"
#include "Pwrd.h"
#include "CryptoUtils.h"
#include <thread>
#include <commctrl.h>
#include <gdiplus.h>
#include <objidl.h>
#include <commdlg.h>
#include <string>
#include <random>
#include <windowsx.h>
#include <shlwapi.h>
#include "trayicon.h"
#include <fstream>
#include <vector>
#include <sstream>
#include <algorithm>

#include <bcrypt.h> 
#include "tinyxml2.h"
#include "CryptoUtils.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shlwapi.lib")

#define MAX_LOADSTRING 100
#define ONE 1


// Global Variables
HINSTANCE hInst = nullptr;
WCHAR szTitle[MAX_LOADSTRING] = { 0 };
WCHAR szWindowClass[MAX_LOADSTRING] = { 0 };
WCHAR exePath[MAX_PATH] = { 0 };
HBITMAP hBitmap = nullptr;
HCURSOR hCustomCursor = nullptr;
HFONT hBigFont = nullptr;
TOOLINFO ti = { 0 };
ULONG_PTR gdiplusToken = 0;
bool dragging = false;
POINT dragStart = { 0, 0 };
int mx = 0, my = 0;
HWND hTooltip = nullptr;
HWND hListView = nullptr, hName = nullptr, hWebsite = nullptr, hEmail = nullptr, hUser = nullptr, hPassword = nullptr, hNote = nullptr, hSearchEdit = nullptr;
HWND hAddBtn = nullptr, hDeleteBtn = nullptr, hSearchBtn = nullptr, hColorBtn = nullptr;
HWND hUpdateBtn = nullptr;
HWND hCopyNameBtn = nullptr, hCopyWebsiteBtn = nullptr, hCopyEmailBtn = nullptr, hCopyUserBtn = nullptr, hCopyPasswordBtn = nullptr, hCopyNoteBtn = nullptr;
HWND AutoBtn = nullptr, XBtn = nullptr, MidBtn = nullptr, LowBtn = nullptr, hBtnicon = nullptr, resetBtn = nullptr;
HWND hStrengthLabel = nullptr, hTogglePasswordBtn = nullptr, hSortCombo = nullptr, hCategory = nullptr, hRestoreBackupBtn = nullptr, hToggleThemeBtn = nullptr;
COLORREF currentColor = RGB(222, 222, 8);
COLORREF dark = RGB(33, 33, 33);
COLORREF textColor = RGB(255, 255, 255);
static bool isDarkTheme = true;
static CHOOSECOLOR cc = { 0 };
static COLORREF customColors[16] = { 0 };
HBRUSH hDarkGreyBrush = nullptr;
HBRUSH butBrush = nullptr;
std::vector<PasswordEntry> entries;
static int g_lastSelectedEntryIndex = -1;
static bool g_dataModifiedInFields = false;
static bool g_isInsideApplyChanges = false;
static bool isPasswordVisible = false;
HWND hToggleStartupBtn = nullptr; // Handle for the new toggle button
bool g_isStartupEnabled = false; // Tracks current startup state

bool DisableStartup();
bool IsStartupEnabled();
bool EnableStartup();

bool updown = true;

static std::wstring g_enteredPassword; // Added for storing the password from the new dialog
static bool g_isInVerifyModeNewDialog = false; // To store the mode for the new dialog

// Registry key for startup
#define REG_RUN_KEY L"Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#define APP_NAME L"Pwrd"

const wchar_t* animationSets[] = {
    L"/-\\|\0",
    L"⠋⠙⠹⠸⠼⠽⠾⠿\0",
    L"←↖↑↗→↘↓↙\0",
    L"█▓▒░▒▓█\0",
    L"⏳⌛\0"
};

const wchar_t* animationChars;
int Tlength = 0;
int animationIndex = 0;
std::vector<BYTE> g_userKeyForXml;
bool g_isPinValidated = false;
static int g_pin_attempts = 0;

// Forward declarations
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK PasswordDlgProc(HWND, UINT, WPARAM, LPARAM);
 


bool LoadXML(std::wstring xmlPath);
void SaveXML();

/*
net stop cryptsvc
net start cryptsvc
sfc /scannow
*/

//std::vector<PasswordEntry> entries;
//bool LoadXML(std::vector<PasswordEntry>& entries, const std::wstring& filePath, const std::vector<BYTE>& key);
//bool SaveXML(const std::vector<PasswordEntry>& entries, const std::wstring& filePath, const std::vector<BYTE>& key);

void PopulateListView();
void CopyToClipboard(const std::wstring& text);
void UpdateListViewColors();
void UpdatePasswordStrength(HWND hWnd, HWND hPasswordEdit);
int cEXIST(HWND hWnd);

INT_PTR CALLBACK PasswordDialogProcNew(HWND, UINT, WPARAM, LPARAM); // Added

const wchar_t* getRandomAnimationSet() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, (sizeof(animationSets) / sizeof(animationSets[0])) - 1);
    size_t index = dist(gen);
    animationChars = animationSets[index];
    Tlength = wcslen(animationChars);
    return animationChars;
}

std::wstring GetFullFilePath(const wchar_t* filename) {
    WCHAR fullPath[MAX_PATH];
    wcscpy_s(fullPath, MAX_PATH, exePath);
    PathAppendW(fullPath, filename);
    return std::wstring(fullPath);
}

void ClearSensitiveDataAndUI(HWND hWnd) {
    if (!g_userKeyForXml.empty()) {
        SecureZeroMemory(g_userKeyForXml.data(), g_userKeyForXml.size());
        g_userKeyForXml.clear();
        g_userKeyForXml.shrink_to_fit();
    }
    g_isPinValidated = false;
    g_pin_attempts = 0;
    entries.clear();
    if (hName) SetWindowTextW(hName, L"");
    if (hWebsite) SetWindowTextW(hWebsite, L"");
    if (hEmail) SetWindowTextW(hEmail, L"");
    if (hUser) SetWindowTextW(hUser, L"");
    if (hPassword) SetWindowTextW(hPassword, L"");
    if (hNote) SetWindowTextW(hNote, L"");
    if (hCategory) SetWindowTextW(hCategory, L"");
    if (hListView) ListView_DeleteAllItems(hListView);
    InvalidateRect(hWnd, NULL, TRUE);
}

void InitializeGDIPlus() {
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
}

void ShutdownGDIPlus() {
    Gdiplus::GdiplusShutdown(gdiplusToken);
}

void Transparent(HWND hWnd, int alpha) {
    SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), (BYTE)alpha, LWA_ALPHA);
}

void GenerateAndSetPassword(HWND hEdit, int length) {
    if (length < 8) length = 8;
    const wchar_t charset[] =
        L"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        L"abcdefghijklmnopqrstuvwxyz"
        L"0123456789"
        L"!@#$%^&*()-_=+[]{}<>?/|";
    const size_t charsetSize = wcslen(charset);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, charsetSize - 1);
    std::wstring password;
    password.reserve(length);
    for (int i = 0; i < length; ++i) {
        password += charset[dist(gen)];
    }
    SetWindowTextW(hEdit, password.c_str());
    UpdatePasswordStrength(hEdit, hEdit);
}






void TestEncryptFile() {
    std::wstring testInput = GetFullFilePath(L"test.txt");
    std::wstring testOutput = GetFullFilePath(L"test_encrypted.bin");
    std::ofstream testFile(testInput, std::ios::binary);
    testFile << "Test data for encryption";
    testFile.close();
    std::wstring password(g_userKeyForXml.begin(), g_userKeyForXml.end());
    if (EncryptFile(testInput, testOutput, password)) {
        MessageBoxW(nullptr, L"Test encryption succeeded.", L"Success", MB_OK);
    }
    else {
        WCHAR msg[256];
        swprintf_s(msg, L"Test encryption failed. Last error: %lu", GetLastError());
        MessageBoxW(nullptr, msg, L"Error", MB_OK | MB_ICONERROR);
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)




{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    INITCOMMONCONTROLSEX icex = { sizeof(INITCOMMONCONTROLSEX) };
    icex.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icex);

    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    PathRemoveFileSpecW(exePath);



    //TestEncryptFile();
    //TestBCrypt();


    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PWRD, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PWRD));
    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PWRD));
    wcex.hCursor = LoadCursor(nullptr, MAKEINTRESOURCE(IDC_CURSOR1));
    wcex.hbrBackground = nullptr;
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    if (!wcex.hIcon || !wcex.hIconSm) {
        MessageBoxW(nullptr, L"Failed to load icons!", L"Error", MB_OK | MB_ICONERROR);
    }

    ATOM atom = RegisterClassExW(&wcex);
    if (atom == 0) {
        WCHAR errorMsg[256];
        swprintf_s(errorMsg, L"Failed to register window class! Error code: %lu", GetLastError());
        MessageBoxW(nullptr, errorMsg, L"Error", MB_OK | MB_ICONERROR);
    }

    return atom;
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;
    HWND hWnd = CreateWindowEx(
        0, szWindowClass, L"Pwrd", WS_POPUP | WS_VISIBLE,
        100, 100, 800, 600, HWND_DESKTOP, nullptr, hInstance, nullptr
    );


    ////////////////////////////////
    ShowWindow(hWnd, SW_HIDE);

   // if (IsStartupEnabled() == true) {
 

  //  }     
   // else {
        int pin = cEXIST(hWnd);
        if (pin != 1) {
            KillTrayIcon();
            ShowWindow(hWnd, SW_HIDE);
            PostQuitMessage(0);
            return FALSE;

        }
  //  }
    //////////////////////////////
  

    if (!hWnd)
    {
        WCHAR errorMsg[256];
        swprintf_s(errorMsg, L"Failed to create window! Error code: %lu", GetLastError());
        MessageBoxW(nullptr, errorMsg, L"Error", MB_OK | MB_ICONERROR);
        return FALSE;
    }



    

    ShowWindow(hWnd, nCmdShow);
    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);
    return TRUE;
}

void AddTooltip(HWND hTooltip, HWND hWnd, HWND hControl, LPCWSTR text)
{
    TOOLINFO ti = { sizeof(TOOLINFO) };
    ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    ti.hwnd = hWnd;
    ti.uId = (UINT_PTR)hControl;
    ti.lpszText = (LPWSTR)text;
    SendMessage(hTooltip, TTM_ADDTOOL, 0, (LPARAM)&ti);
}

void ApplyEntryChanges(HWND hWnd, int entryIndex) {
    if (entryIndex < 0 || static_cast<size_t>(entryIndex) >= entries.size()) {
        return;
    }

    PasswordEntry updatedEntry;
    WCHAR buffer[2048];

    GetWindowTextW(hName, buffer, ARRAYSIZE(buffer));
    updatedEntry.name = buffer;
    GetWindowTextW(hWebsite, buffer, ARRAYSIZE(buffer));
    updatedEntry.website = buffer;
    GetWindowTextW(hEmail, buffer, ARRAYSIZE(buffer));
    updatedEntry.email = buffer;
    GetWindowTextW(hUser, buffer, ARRAYSIZE(buffer));
    updatedEntry.user = buffer;
    GetWindowTextW(hPassword, buffer, ARRAYSIZE(buffer));
    updatedEntry.password = buffer;
    GetWindowTextW(hNote, buffer, ARRAYSIZE(buffer));
    updatedEntry.note = buffer;
    GetWindowTextW(hCategory, buffer, ARRAYSIZE(buffer));
    updatedEntry.category = buffer;
    updatedEntry.color = currentColor;

    entries[entryIndex] = updatedEntry;
    g_lastSelectedEntryIndex = entryIndex;

    SaveXML();
    PopulateListView();
}

void UpdatePasswordStrength(HWND hWnd, HWND hPasswordEdit) {
    WCHAR buffer[1024];
    GetWindowTextW(hPasswordEdit, buffer, 1024);
    std::wstring password = buffer;
    int score = 0;
    if (password.length() >= 8) score++;
    if (std::any_of(password.begin(), password.end(), ::iswupper)) score++;
    if (std::any_of(password.begin(), password.end(), ::iswlower)) score++;
    if (std::any_of(password.begin(), password.end(), ::iswdigit)) score++;
    if (std::any_of(password.begin(), password.end(), [](wchar_t c) { return wcschr(L"!@#$%^&*()-_=+[]{}<>?/|", c); })) score++;
    const wchar_t* strength = score <= 2 ? L"Weak" : score <= 4 ? L"Medium" : L"Strong";
    COLORREF strengthColor = score <= 2 ? RGB(255, 0, 0) : score <= 4 ? RGB(255, 165, 0) : RGB(0, 255, 0);
    SetWindowTextW(hStrengthLabel, strength);
    //SendMessage(hStrengthLabel, WM_SETFONT, (WPARAM)hBigFont, TRUE);
    SetBkMode(GetDC(hStrengthLabel), TRANSPARENT);
    InvalidateRect(hStrengthLabel, nullptr, TRUE);
}

void ini(HWND hWnd)
{


   


    animationChars = getRandomAnimationSet();


    hListView = CreateWindowEx(0, WC_LISTVIEW, L"",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
        10, 10, 200, 580, hWnd, (HMENU)IDC_LISTVIEW, hInst, nullptr);

    LVCOLUMNW lvc = { 0 };
    lvc.mask = LVCF_TEXT | LVCF_WIDTH;
    lvc.cx = 200;
    lvc.pszText = (LPWSTR)L"";
    ListView_InsertColumn(hListView, 0, &lvc);
    ListView_SetExtendedListViewStyle(hListView, LVS_EX_FULLROWSELECT);
    ShowScrollBar(hListView, SB_VERT, FALSE);

    HWND hHeader = ListView_GetHeader(hListView);
    ListView_SetBkColor(hListView, dark);
    ListView_SetTextBkColor(hListView, dark);
    ListView_SetTextColor(hListView, textColor);

    // Name field
    CreateWindow(L"STATIC", L"Name", WS_CHILD | WS_VISIBLE,
        220, 18, 80, 22, hWnd, nullptr, hInst, nullptr);
    hName = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        310, 18, 330, 26, hWnd, (HMENU)IDC_NAME, hInst, nullptr);
    hCopyNameBtn = CreateWindow(L"BUTTON", L"Copy", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        654, 18, 50, 22, hWnd, (HMENU)IDC_COPY_NAME, hInst, nullptr);

    // Website field (increased spacing: 50 from previous)
    CreateWindow(L"STATIC", L"Website", WS_CHILD | WS_VISIBLE,
        220, 60, 80, 20, hWnd, nullptr, hInst, nullptr);
    hWebsite = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        310, 60, 330, 28, hWnd, (HMENU)IDC_WEBSITE, hInst, nullptr);
    hCopyWebsiteBtn = CreateWindow(L"BUTTON", L"Copy", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        654, 60, 50, 20, hWnd, (HMENU)IDC_COPY_WEBSITE, hInst, nullptr);

    // Email field (increased spacing: 50 from previous)
    CreateWindow(L"STATIC", L"Email", WS_CHILD | WS_VISIBLE,
        220, 110, 80, 20, hWnd, nullptr, hInst, nullptr);
    hEmail = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        310, 110, 330, 28, hWnd, (HMENU)IDC_EMAIL, hInst, nullptr);
    hCopyEmailBtn = CreateWindow(L"BUTTON", L"Copy", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        654, 110, 50, 20, hWnd, (HMENU)IDC_COPY_EMAIL, hInst, nullptr);

    // User field (increased spacing: 50 from previous)
    CreateWindow(L"STATIC", L"User", WS_CHILD | WS_VISIBLE,
        220, 160, 80, 20, hWnd, nullptr, hInst, nullptr);
    hUser = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        310, 160, 330, 28, hWnd, (HMENU)IDC_USER, hInst, nullptr);
    hCopyUserBtn = CreateWindow(L"BUTTON", L"Copy", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        654, 160, 50, 20, hWnd, (HMENU)IDC_COPY_USER, hInst, nullptr);

    // Password field (increased spacing: 50 from previous)
    CreateWindow(L"STATIC", L"Password", WS_CHILD | WS_VISIBLE,
        220, 210, 80, 20, hWnd, nullptr, hInst, nullptr);
     AutoBtn = CreateWindow(L"BUTTON", L"Auto", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        236, 232, 40, 20, hWnd, (HMENU)IDC_AutoBtn, hInst, nullptr);
   
   // hTogglePasswordBtn = CreateWindow(L"BUTTON", L"Show", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
   //     570, 212, 70, 28, hWnd, (HMENU)IDC_TOGGLE_PASSWORD, hInst, nullptr);

   // hPassword = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_PASSWORD,
   //    310, 210, 250, 28, hWnd, (HMENU)IDC_PASSWORD, hInst, nullptr);

    hPassword = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL  ,
        310, 210, 250, 28, hWnd, (HMENU)IDC_PASSWORD, hInst, nullptr);


    hCopyPasswordBtn = CreateWindow(L"BUTTON", L"Copy", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        654, 210, 50, 20, hWnd, (HMENU)IDC_COPY_PASSWORD, hInst, nullptr);
    hStrengthLabel = CreateWindow(L"STATIC", L"Strength: ", WS_CHILD | WS_VISIBLE,
        310, 250, 100, 30, hWnd, (HMENU)IDC_STRENGTH_LABEL, hInst, nullptr);

    // Category field (increased spacing: 60 from previous to avoid overlap)
    CreateWindow(L"STATIC", L"Category", WS_CHILD | WS_VISIBLE,
        220, 300, 80, 20, hWnd, nullptr, hInst, nullptr);
    hCategory = CreateWindow(L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWN | CBS_AUTOHSCROLL,
        300, 300, 350, 200, hWnd, (HMENU)IDC_CATEGORY, hInst, nullptr);
    SendMessage(hCategory, CB_ADDSTRING, 0, (LPARAM)L"Personal");
    SendMessage(hCategory, CB_ADDSTRING, 0, (LPARAM)L"Work");
    SendMessage(hCategory, CB_ADDSTRING, 0, (LPARAM)L"Web");
    SendMessage(hCategory, CB_ADDSTRING, 0, (LPARAM)L"Email");
    SendMessage(hCategory, CB_ADDSTRING, 0, (LPARAM)L"Crap");
    SendMessage(hCategory, CB_ADDSTRING, 0, (LPARAM)L"Other");
    SetWindowTextW(hCategory, L"Web");

    // Note field (increased spacing: 50 from previous to avoid overlap)
    CreateWindow(L"STATIC", L"Note", WS_CHILD | WS_VISIBLE,
        220, 340, 60, 20, hWnd, nullptr, hInst, nullptr);
    hNote = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
        300, 340, 350, 100, hWnd, (HMENU)IDC_NOTE, hInst, nullptr);
    hCopyNoteBtn = CreateWindow(L"BUTTON", L"Copy", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        654, 340, 50, 20, hWnd, (HMENU)IDC_COPY_NOTE, hInst, nullptr);

    // Buttons (repositioned lower with more space after password)
    hUpdateBtn = CreateWindow(L"BUTTON", L"Update", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        474, 450, 80, 30, hWnd, (HMENU)IDC_UPDATE_BUTTON, hInst, nullptr);
    hAddBtn = CreateWindow(L"BUTTON", L"Add", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        564, 450, 80, 30, hWnd, (HMENU)IDC_ADD, hInst, nullptr);
    hDeleteBtn = CreateWindow(L"BUTTON", L"<-Delete", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        220, 450, 80, 30, hWnd, (HMENU)IDC_DELETE, hInst, nullptr);

    hSearchBtn = CreateWindow(L"BUTTON", L"Search", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        474, 500, 80, 20, hWnd, (HMENU)IDC_SEARCH, hInst, nullptr);
    resetBtn = CreateWindow(L"BUTTON", L"Reset", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        564, 500, 80, 20, hWnd, (HMENU)IDC_reset, hInst, nullptr);
    hSearchEdit = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        220, 500, 240, 20, hWnd, (HMENU)IDC_SEARCH_EDIT, hInst, nullptr);

   // hSortCombo = CreateWindow(L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
      // 220, 530, 120, 100, hWnd, (HMENU)IDC_SORT_COMBO, hInst, nullptr);

    hSortCombo = CreateWindow(L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWN | CBS_AUTOHSCROLL,
        220, 530, 120, 100, hWnd, (HMENU)IDC_SORT_COMBO, hInst, nullptr);
    SendMessage(hSortCombo, CB_ADDSTRING, 0, (LPARAM)L"Sort by Name");
    SendMessage(hSortCombo, CB_ADDSTRING, 0, (LPARAM)L"Sort by Category");
    SendMessage(hSortCombo, CB_SETCURSEL, 0, 0);

    CreateWindow(L"STATIC", L"Color:", WS_CHILD | WS_VISIBLE,
        350, 530, 50, 20, hWnd, nullptr, hInst, nullptr);
    hColorBtn = CreateWindow(L"BUTTON", L"Pick Color", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        410, 530, 80, 20, hWnd, (HMENU)IDC_COLOR, hInst, nullptr);

    hRestoreBackupBtn = CreateWindow(L"BUTTON", L"Restore", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        220, 565, 60, 20, hWnd, (HMENU)IDC_RESTORE_BACKUP, hInst, nullptr);
    hToggleThemeBtn = CreateWindow(L"BUTTON", L"Toggle Theme", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        330, 565, 100, 20, hWnd, (HMENU)IDC_TOGGLE_THEME, hInst, nullptr);

    // Window control buttons
    XBtn = CreateWindow(L"BUTTON", L"X", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        774, 5, 22, 30, hWnd, (HMENU)IDC_XBtn, hInst, nullptr);
    MidBtn = CreateWindow(L"BUTTON", L"O", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        774, 35, 22, 30, hWnd, (HMENU)IDC_MidBtn, hInst, nullptr);
    LowBtn = CreateWindow(L"BUTTON", L"-", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        774, 65, 22, 30, hWnd, (HMENU)IDC_LowBtn, hInst, nullptr);


    // New toggle startup button
    g_isStartupEnabled = IsStartupEnabled(); // Check initial state
    hToggleStartupBtn = CreateWindow(L"BUTTON",
        g_isStartupEnabled ? L"Run at Startup: ON" : L"Run at Startup: OFF",
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        440, 565, 140, 20, hWnd, (HMENU)IDC_TOGGLE_STARTUP, hInst, nullptr);



    // Icon button
    HICON tmphIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_PWRD));
    ICONINFO iconInfo;
    GetIconInfo(tmphIcon, &iconInfo);
    hBtnicon = CreateWindowEx(
        0, L"BUTTON", nullptr,
        WS_CHILD | WS_VISIBLE | BS_ICON,
        760, 560, iconInfo.xHotspot * 2, iconInfo.yHotspot * 2,
        hWnd, (HMENU)1001, hInst, nullptr
    );
    SendMessage(hBtnicon, BM_SETIMAGE, IMAGE_ICON, (LPARAM)tmphIcon);

    // Tooltip setup
    hTooltip = CreateWindowEx(0, TOOLTIPS_CLASS, nullptr,
        WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        hWnd, nullptr, hInst, nullptr);

    if (hTooltip)
    {
        SetWindowPos(hTooltip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        AddTooltip(hTooltip, hWnd, hName, L"Enter the name of the entry");
        AddTooltip(hTooltip, hWnd, hWebsite, L"Enter the website URL");
        AddTooltip(hTooltip, hWnd, hEmail, L"Enter the email address");
        AddTooltip(hTooltip, hWnd, hUser, L"Enter the username");
        AddTooltip(hTooltip, hWnd, hPassword, L"Enter the password");
        AddTooltip(hTooltip, hWnd, hNote, L"Enter additional notes");
        AddTooltip(hTooltip, hWnd, hCategory, L"Select or enter a category");
        AddTooltip(hTooltip, hWnd, hSearchEdit, L"Enter search term");
        AddTooltip(hTooltip, hWnd, hAddBtn, L"Add a new entry");
        AddTooltip(hTooltip, hWnd, hDeleteBtn, L"Delete the selected entry");
        AddTooltip(hTooltip, hWnd, hUpdateBtn, L"Update the selected entry");
        AddTooltip(hTooltip, hWnd, hSearchBtn, L"Search entries by any field");
        AddTooltip(hTooltip, hWnd, hColorBtn, L"Pick a color for the entry");
        AddTooltip(hTooltip, hWnd, hCopyNameBtn, L"Copy name to clipboard");
        AddTooltip(hTooltip, hWnd, hCopyWebsiteBtn, L"Copy website to clipboard");
        AddTooltip(hTooltip, hWnd, hCopyEmailBtn, L"Copy email to clipboard");
        AddTooltip(hTooltip, hWnd, hCopyUserBtn, L"Copy username to clipboard");
        AddTooltip(hTooltip, hWnd, hCopyPasswordBtn, L"Copy password to clipboard");
        AddTooltip(hTooltip, hWnd, hCopyNoteBtn, L"Copy note to clipboard");
        AddTooltip(hTooltip, hWnd, AutoBtn, L"Generate a random password");
        AddTooltip(hTooltip, hWnd, hTogglePasswordBtn, L"Toggle password visibility");
        AddTooltip(hTooltip, hWnd, hSortCombo, L"Sort entries by name or category");
        AddTooltip(hTooltip, hWnd, hRestoreBackupBtn, L"Restore from backup file");
        AddTooltip(hTooltip, hWnd, hToggleThemeBtn, L"Switch between dark and light themes");
        AddTooltip(hTooltip, hWnd, hToggleStartupBtn, L"Toggle whether the application starts with Windows");


        SendMessage(hTooltip, TTM_SETMAXTIPWIDTH, 0, 100);
        SendMessage(hTooltip, TTM_SETDELAYTIME, TTDT_AUTOPOP, 1000);
    }

    // Font setup
    NONCLIENTMETRICS ncm = { sizeof(NONCLIENTMETRICS) };
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
    ncm.lfMessageFont.lfHeight = 20;
    HFONT hFont = CreateFontIndirect(&ncm.lfMessageFont);
    SendMessage(hName, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hWebsite, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hEmail, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hUser, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hPassword, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hNote, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hCategory, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hSearchEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hStrengthLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
    ncm.lfMessageFont.lfHeight = 24;
    HFONT hFont2 = CreateFontIndirect(&ncm.lfMessageFont);
    SendMessage(hListView, WM_SETFONT, (WPARAM)hFont2, TRUE);
    ncm.lfMessageFont.lfHeight = 40;
    hBigFont = CreateFontIndirect(&ncm.lfMessageFont);

    // Color chooser setup
    ZeroMemory(&cc, sizeof(cc));
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = hWnd;
    cc.lpCustColors = customColors;
    cc.Flags = CC_FULLOPEN | CC_RGBINIT;

    // Initialize flags
    g_lastSelectedEntryIndex = -1;
    g_dataModifiedInFields = false;

    SetTimer(hWnd, ONE, 333, nullptr);
    
    //LoadXML(L"data.xml");
    PopulateListView();
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HINSTANCE hInstance_WndProc = NULL;
    static bool isMinimizedState = false;
    static HWND hHeader = nullptr;

    switch (message)
    {
    case WM_CREATE:
    {
        InitCommonControls();
        CoInitialize(NULL);
        OleInitialize(NULL);
        InitializeGDIPlus();
        hInstance_WndProc = ((LPCREATESTRUCT)lParam)->hInstance;
        hDarkGreyBrush = CreateSolidBrush(dark);
        butBrush = CreateSolidBrush(RGB(66, 66, 66));
        hBitmap = LoadBitmap(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDB_BITMAP1));
        if (!hBitmap) {
            MessageBoxW(hWnd, L"Failed to load bitmap!", L"Error", MB_OK | MB_ICONERROR);
        }
        hCustomCursor = LoadCursor(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDC_CURSOR1));
        if (!hCustomCursor) {
            MessageBoxW(hWnd, L"Failed to load cursor!", L"Error", MB_OK | MB_ICONERROR);
        }

        ShowWindow(hWnd, SW_HIDE);
        ini(hWnd);
        ShowScrollBar(hListView, SB_VERT, FALSE);
        ShowScrollBar(hListView, SB_HORZ, FALSE);
        hHeader = ListView_GetHeader(hListView);
        CreateTrayIcon(hWnd, hInstance_WndProc, IDI_PWRD);
         
        
        return 0;
    }

    case WM_TRAYICON:
    {
        if (lParam == WM_LBUTTONUP || lParam == WM_LBUTTONDBLCLK)
        {
            int pin = cEXIST(hWnd);
            if (pin == 1) {
                ShowWindow(hWnd, SW_SHOW);
                ShowWindow(hWnd, SW_RESTORE);
                SetForegroundWindow(hWnd);
                //LoadXML(GetFullFilePath(L"data.xml")); // Use full path
            }
            return 0;
        }
        else if (lParam == WM_RBUTTONUP)
        {
            POINT pt;
            GetCursorPos(&pt);
            SetForegroundWindow(hWnd);
            HMENU hMenu = CreatePopupMenu();
            if (hMenu)
            {
                if (!updown)
                   AppendMenu(hMenu, MF_STRING, IDM_SHOW, L"Restore");
                AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
                AppendMenu(hMenu, MF_STRING, IDM_EXIT, L"Exit"); // Use defined IDs
                int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY | TPM_RIGHTBUTTON,
                    pt.x, pt.y, 0, hWnd, NULL);
                switch (cmd)
                {
                case IDM_SHOW:
                {
                    int pin = cEXIST(hWnd);
                    if (pin == 1) {
                        ShowWindow(hWnd, SW_SHOW);
                        ShowWindow(hWnd, SW_RESTORE);
                        SetForegroundWindow(hWnd);
                        //LoadXML(GetFullFilePath(L"data.xml"));
                        updown = true;
                    }
                    break;
                }
                case IDM_EXIT:
                    KillTrayIcon(); // Ensure tray icon is removed
                    DestroyWindow(hWnd);
                    break;
                }
                DestroyMenu(hMenu);
            }
            return 0;
        }
        break;
    }

    case WM_SHOWWINDOW:
    {
        ShowWindow(hTooltip, SW_SHOW);
        hHeader = ListView_GetHeader(hListView);
        if (hHeader)
            ShowWindow(hHeader, SW_HIDE);
        break;
    }

    case WM_DRAWITEM:
    {
        LPDRAWITEMSTRUCT lpDraw = (LPDRAWITEMSTRUCT)lParam;
        COLORREF Ti = isDarkTheme ? RGB(177, 177, 177) : RGB(0, 0, 0);
        switch (lpDraw->CtlID)
        {
        case IDC_UPDATE_BUTTON:
        case IDC_reset:
        case IDC_XBtn:
        case IDC_MidBtn:
        case IDC_LowBtn:
        case IDC_AutoBtn:
        case IDC_DELETE:
        case IDC_ADD:
        case IDC_COLOR:
        case IDC_TOGGLE_STARTUP:
        case IDC_SEARCH:
        case IDC_TOGGLE_PASSWORD:
        case IDC_RESTORE_BACKUP:
        case IDC_TOGGLE_THEME:
        case IDC_COPY_NAME:
        case IDC_COPY_WEBSITE:
        case IDC_COPY_EMAIL:
        case IDC_COPY_USER:
        case IDC_COPY_PASSWORD:
        case IDC_COPY_NOTE:
        {
            HDC hdc = lpDraw->hDC;
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, lpDraw->CtlID == IDC_XBtn || lpDraw->CtlID == IDC_MidBtn || lpDraw->CtlID == IDC_LowBtn ? RGB(222, 222, 18) : Ti);
            FillRect(hdc, &lpDraw->rcItem, hDarkGreyBrush);
            FrameRect(hdc, &lpDraw->rcItem, (HBRUSH)GetStockObject(BLACK_BRUSH));
            const wchar_t* buttonText = L"";
            switch (lpDraw->CtlID)
            {
            case IDC_UPDATE_BUTTON: buttonText = L"Update"; break;
            case IDC_reset: buttonText = L"Reset"; break;
            case IDC_XBtn: buttonText = L"X"; break;
            case IDC_MidBtn: buttonText = L"O"; break;
            case IDC_LowBtn: buttonText = L"-"; break;
            case IDC_AutoBtn: buttonText = L"Auto"; break;
            case IDC_DELETE: buttonText = L"<-Delete"; break;
            case IDC_ADD: buttonText = L"Add"; break;
            case IDC_COLOR: buttonText = L"Pick Color"; break;
            case IDC_SEARCH: buttonText = L"Search"; break;
            //case IDC_TOGGLE_PASSWORD: buttonText = isPasswordVisible ? L"Hide" : L"Show"; break;
            case IDC_RESTORE_BACKUP: buttonText = L"Restor"; break;
            case IDC_TOGGLE_THEME: buttonText = isDarkTheme ? L"Dark2 Theme" : L"Dark Theme"; break;
            case IDC_TOGGLE_STARTUP: buttonText = g_isStartupEnabled ? L"Run at Startup: ON" : L"Run at Startup: OFF"; break;
            case IDC_COPY_NAME:
            case IDC_COPY_WEBSITE:
            case IDC_COPY_EMAIL:
            case IDC_COPY_USER:
            case IDC_COPY_PASSWORD:
            case IDC_COPY_NOTE: buttonText = L"Copy"; break;
            }
            DrawText(hdc, buttonText, -1, &lpDraw->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            hHeader = ListView_GetHeader(hListView);
            ShowWindow(hHeader, SW_HIDE);
            return TRUE;
        }
        }
        break;
    }

    case WM_CTLCOLORSTATIC:
    {
        HDC hdcStatic = (HDC)wParam;
        SetBkMode(hdcStatic, TRANSPARENT);
        SetTextColor(hdcStatic, textColor);
        return (INT_PTR)hDarkGreyBrush;
    }

    case WM_CTLCOLORDLG:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORSCROLLBAR:
    case WM_CTLCOLOREDIT:
    
    {
        HDC hdc = (HDC)wParam;
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, currentColor);
        ShowScrollBar(hListView, SB_VERT, FALSE);
        ShowScrollBar(hListView, SB_HORZ, FALSE);
        hHeader = ListView_GetHeader(hListView);
        ShowWindow(hHeader, SW_HIDE);
        return (LRESULT)hDarkGreyBrush;
    }

    case WM_CTLCOLORBTN:
    {
        HDC hdc = (HDC)wParam;
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, isDarkTheme ? RGB(255, 255, 255) : RGB(0, 0, 0));
        return (LRESULT)hDarkGreyBrush;
    }

    case WM_ERASEBKGND:
    {
        HDC hdc = (HDC)wParam;
        HBRUSH hBrush = CreateSolidBrush(dark);
        RECT rect;
        GetClientRect(hWnd, &rect);
        FillRect(hdc, &rect, hBrush);
        DeleteObject(hBrush);
        ShowWindow(hHeader, SW_HIDE);
        SendMessage(hNote, WM_SETREDRAW, TRUE, 0);
        InvalidateRect(hNote, NULL, TRUE);
        return 1;
    }

    case WM_SYSCOLORCHANGE:
    {
        InvalidateRect(hWnd, NULL, TRUE);
        ShowWindow(hHeader, SW_HIDE);
        return 0;
    }

    case WM_NOTIFY:
    {
        LPNMHDR pnm = (LPNMHDR)lParam;
        if (pnm->idFrom == IDC_LISTVIEW)
        {
            if (pnm->code == LVN_ITEMCHANGED)
            {
                NMLISTVIEW* pnmv = (NMLISTVIEW*)lParam;
                if (pnmv->uNewState & LVIS_SELECTED)
                {
                    int idx = pnmv->iItem;
                    if (idx >= 0 && static_cast<size_t>(idx) < entries.size())
                    {
                        if (g_dataModifiedInFields && g_lastSelectedEntryIndex != -1 &&
                            static_cast<size_t>(g_lastSelectedEntryIndex) < entries.size() &&
                            !g_isInsideApplyChanges)
                        {
                            g_isInsideApplyChanges = true;
                            ApplyEntryChanges(hWnd, g_lastSelectedEntryIndex);
                            g_isInsideApplyChanges = false;
                        }

                        SetWindowTextW(hName, entries[idx].name.c_str());
                        SetWindowTextW(hWebsite, entries[idx].website.c_str());
                        SetWindowTextW(hEmail, entries[idx].email.c_str());
                        SetWindowTextW(hUser, entries[idx].user.c_str());
                        SetWindowTextW(hPassword, entries[idx].password.c_str());
                        SetWindowTextW(hNote, entries[idx].note.c_str());
                        SetWindowTextW(hCategory, entries[idx].category.c_str());
                        currentColor = entries[idx].color;
                        g_lastSelectedEntryIndex = idx;
                        g_dataModifiedInFields = false;
                        UpdatePasswordStrength(hWnd, hPassword);
                    }
                }
            }
            else if (pnm->code == NM_CUSTOMDRAW)
            {
                LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)lParam;
                switch (lplvcd->nmcd.dwDrawStage)
                {
                case CDDS_PREPAINT:
                    return CDRF_NOTIFYITEMDRAW;
                case CDDS_ITEMPREPAINT:
                {
                    LVITEMW lvi = { 0 };
                    lvi.iItem = (int)lplvcd->nmcd.dwItemSpec;
                    lvi.mask = LVIF_PARAM;
                    ListView_GetItem(hListView, &lvi);
                    size_t idx = (size_t)lvi.lParam;
                    if (idx < entries.size())
                    {
                        lplvcd->clrText = entries[idx].color;
                    }
                    lplvcd->clrTextBk = dark;
                    return CDRF_DODEFAULT;
                }
                }
                return CDRF_DODEFAULT;
            }
        }
        return 0;
    }

    case WM_MOUSEWHEEL:
    {
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        ScreenToClient(hListView, &pt);
        RECT rc;
        GetClientRect(hListView, &rc);
        if (PtInRect(&rc, pt)) {
            SendMessage(hListView, WM_MOUSEWHEEL, wParam, lParam);
        }
        ShowScrollBar(hListView, SB_VERT, FALSE);
        ShowScrollBar(hListView, SB_HORZ, FALSE);
        hHeader = ListView_GetHeader(hListView);
        ShowWindow(hHeader, SW_HIDE);
        return 0;
    }

    case WM_MOUSEMOVE:
    {
        mx = LOWORD(lParam);
        my = HIWORD(lParam);
        if (dragging)
        {
            POINT pt;
            GetCursorPos(&pt);
            SetWindowPos(hWnd, nullptr, pt.x - dragStart.x, pt.y - dragStart.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
        }
        return 0;
    }

    case WM_LBUTTONDOWN:
    {
        dragging = true;
        POINT pt;
        GetCursorPos(&pt);
        RECT rect;
        GetWindowRect(hWnd, &rect);
        dragStart.x = pt.x - rect.left;
        dragStart.y = pt.y - rect.top;
        SetCapture(hWnd);
        return 0;
    }

    case WM_LBUTTONUP:
    {
        dragging = false;
        ReleaseCapture();
        return 0;
    }

    case WM_TIMER:
    {
        if (wParam == ONE) {
            animationIndex = (animationIndex + 1) % Tlength;
            RECT rect = { 620, 160, 720, 560 };
            InvalidateRect(hWnd, &rect, TRUE);
        }
        break;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        HPEN hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
        HGDIOBJ hOldPen = SelectObject(hdc, hPen);
        RECT clientRect;
        GetClientRect(hWnd, &clientRect);
        int width = clientRect.right - clientRect.left;
        int height = clientRect.bottom - clientRect.top;
        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hbmMem = CreateCompatibleBitmap(hdc, width, height);
        HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);
        HBRUSH hBrush = CreateSolidBrush(dark);
        FillRect(hdcMem, &clientRect, hBrush);
        DeleteObject(hBrush);

        if (hBitmap)
        {
            HDC hdcBitmap = CreateCompatibleDC(hdcMem);
            HBITMAP hbmOldBitmap = (HBITMAP)SelectObject(hdcBitmap, hBitmap);
            BITMAP bitmap;
            GetObject(hBitmap, sizeof(BITMAP), &bitmap);
            BitBlt(hdcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcBitmap, 0, 0, SRCCOPY);
            SelectObject(hdcBitmap, hbmOldBitmap);
            DeleteDC(hdcBitmap);
        }
        else {
            TextOutW(hdcMem, 10, 10, L"Bitmap not loaded", 16);
        }


        /////////////////////////////////////
        BitBlt(hdc, 0, 0, width, height, hdcMem, 0, 0, SRCCOPY);
        HFONT hOldFont = (HFONT)SelectObject(hdc, hBigFont);
        wchar_t currentChar[2] = { animationChars[animationIndex], L'\0' };
        SetTextColor(hdc, isDarkTheme ? RGB(11, 11, 11) : RGB(200, 200, 200));
        SetBkMode(hdc, TRANSPARENT);
        TextOut(hdc, 670, 250, currentChar, 1);
        /////////////////////////////////////////

        HPEN hPen2 = CreatePen(PS_SOLID, 1, isDarkTheme ? RGB(22, 22, 22) : RGB(100, 100, 100));
        HGDIOBJ hOldPen2 = SelectObject(hdc, hPen2);
        HBRUSH hBrush2 = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
        HGDIOBJ hOldBrush2 = SelectObject(hdc, hBrush2);
        Rectangle(hdc, 0, 0, 800, 600);
        hPen2 = CreatePen(PS_SOLID, 1, isDarkTheme ? RGB(11, 11, 11) : RGB(150, 150, 150));
        Rectangle(hdc, 1, 1, 799, 599);
        SelectObject(hdc, hOldPen2);
        SelectObject(hdc, hOldBrush2);
        DeleteObject(hPen2);

        SelectObject(hdc, hOldFont);
        SelectObject(hdcMem, hbmOld);
        DeleteObject(hbmMem);
        DeleteDC(hdcMem);
        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        int wmEvent = HIWORD(wParam);

        switch (wmId)
        {
        case IDC_NAME:
        case IDC_WEBSITE:
        case IDC_EMAIL:
        case IDC_USER:
        case IDC_PASSWORD:
        case IDC_NOTE:
        case IDC_CATEGORY:
        case IDC_SEARCH_EDIT:
            if (wmEvent == EN_CHANGE)
            {
                g_dataModifiedInFields = true;
                if (wmId == IDC_PASSWORD) {
                    UpdatePasswordStrength(hWnd, hPassword);
                }
            }
            break;


        case IDC_TOGGLE_STARTUP:
        {
            g_isStartupEnabled = !g_isStartupEnabled;
            if (g_isStartupEnabled) {
                if (EnableStartup()) {
                    SetWindowTextW(hToggleStartupBtn, L"Run at Startup: ON");
                }
                else {
                    g_isStartupEnabled = false; // Revert state on failure
                    MessageBoxW(hWnd, L"Failed to enable startup.", L"Error", MB_OK | MB_ICONERROR);
                }
            }
            else {
                if (DisableStartup()) {
                    SetWindowTextW(hToggleStartupBtn, L"Run at Startup: OFF");
                }
                else {
                    g_isStartupEnabled = true; // Revert state on failure
                    MessageBoxW(hWnd, L"Failed to disable startup.", L"Error", MB_OK | MB_ICONERROR);
                }
            }
            InvalidateRect(hToggleStartupBtn, nullptr, TRUE); // Redraw button
            break;
        }


        case IDC_SORT_COMBO:
            if (wmEvent == CBN_SELCHANGE)
            {
                int sel = SendMessage(hSortCombo, CB_GETCURSEL, 0, 0);
                if (sel == 0) { // Sort by Name
                    std::sort(entries.begin(), entries.end(), [](const PasswordEntry& a, const PasswordEntry& b) {
                        return _wcsicmp(a.name.c_str(), b.name.c_str()) < 0;
                        });
                }
                else if (sel == 1) { // Sort by Category
                    std::sort(entries.begin(), entries.end(), [](const PasswordEntry& a, const PasswordEntry& b) {
                        return _wcsicmp(a.category.c_str(), b.category.c_str()) < 0;
                        });
                }
                PopulateListView();
            }
            break;
        case IDC_XBtn:
            KillTrayIcon();
            DestroyWindow(hWnd);
            break;
        case IDC_MidBtn:

             
            ShowTrayIcon();
            updown = false;
            ShowWindow(hWnd, SW_HIDE);
        
            break;
        case IDC_LowBtn:
            SendMessage(hWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
            break;
        case IDC_AutoBtn:
            GenerateAndSetPassword(hPassword, 16);
            g_dataModifiedInFields = true;
            UpdatePasswordStrength(hWnd, hPassword);
            break;
            
            /*
        case IDC_TOGGLE_PASSWORD:
        {
            isPasswordVisible = !isPasswordVisible;
            SetWindowTextW(hTogglePasswordBtn, isPasswordVisible ? L"Hide" : L"Show");

            // Store current password text
            WCHAR buffer[1024];
            GetWindowTextW(hPassword, buffer, 1024);

            // Toggle password style
            LONG style = GetWindowLong(hPassword, GWL_STYLE);
            if (isPasswordVisible) {
                style &= ~ES_PASSWORD; // Remove password masking
            }
            else {
                style |= ES_PASSWORD;  // Add password masking
            }
            SetWindowLong(hPassword, GWL_STYLE, style);

            // Restore text and force redraw
            SetWindowTextW(hPassword, buffer); // Restore original text
            InvalidateRect(hPassword, nullptr, TRUE);
            UpdateWindow(hPassword);
            RedrawWindow(hPassword, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
            break;
        }
        */

        case IDC_ADD:
        {
            PasswordEntry entry;
            WCHAR buffer[1024];
            GetWindowTextW(hName, buffer, 1024);
            entry.name = buffer;
            GetWindowTextW(hWebsite, buffer, 1024);
            entry.website = buffer;
            GetWindowTextW(hEmail, buffer, 1024);
            entry.email = buffer;
            GetWindowTextW(hUser, buffer, 1024);
            entry.user = buffer;
            GetWindowTextW(hPassword, buffer, 1024);
            entry.password = buffer;
            GetWindowTextW(hNote, buffer, 1024);
            entry.note = buffer;
            GetWindowTextW(hCategory, buffer, 1024);
            entry.category = buffer;
            entry.color = currentColor;

            if (!entry.name.empty()) {
                bool exists = false;
                for (const auto& e : entries) {
                    if (_wcsicmp(e.name.c_str(), entry.name.c_str()) == 0) {
                        exists = true;
                        MessageBoxW(hWnd, L"An entry with this name already exists. Please use a unique name or update the existing entry.", L"Error", MB_OK | MB_ICONWARNING);
                        break;
                    }
                }
                if (!exists) {
                    entries.push_back(entry);
                    SaveXML();
                    PopulateListView();
                    SetWindowTextW(hName, L"");
                    SetWindowTextW(hWebsite, L"");
                    SetWindowTextW(hEmail, L"");
                    SetWindowTextW(hUser, L"");
                    SetWindowTextW(hPassword, L"");
                    SetWindowTextW(hNote, L"");
                    SetWindowTextW(hCategory, L"Personal");
                    currentColor = RGB(53, 50, 50);
                    g_lastSelectedEntryIndex = -1;
                    g_dataModifiedInFields = false;
                    UpdatePasswordStrength(hWnd, hPassword);
                }
                SetFocus(hWnd);
            }
            else {
                MessageBoxW(hWnd, L"Name field cannot be empty.", L"Error", MB_OK | MB_ICONWARNING);
            }
            break;
        }
        case IDC_UPDATE_BUTTON:
        {
            if (g_lastSelectedEntryIndex != -1 && static_cast<size_t>(g_lastSelectedEntryIndex) < entries.size()) {
                g_isInsideApplyChanges = true;
                ApplyEntryChanges(hWnd, g_lastSelectedEntryIndex);
                g_isInsideApplyChanges = false;
            }
            break;
        }
        case IDC_DELETE:
        {
            int idx = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
            if (idx >= 0 && static_cast<size_t>(idx) < entries.size()) {
                WCHAR message[256];
                swprintf_s(message, L"Are you sure you want to delete the entry '%s'?", entries[idx].name.c_str());
                if (MessageBoxW(hWnd, message, L"Confirm Delete", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                    entries.erase(entries.begin() + idx);
                    SaveXML();
                    PopulateListView();
                    SetWindowTextW(hName, L"");
                    SetWindowTextW(hWebsite, L"");
                    SetWindowTextW(hEmail, L"");
                    SetWindowTextW(hUser, L"");
                    SetWindowTextW(hPassword, L"");
                    SetWindowTextW(hNote, L"");
                    SetWindowTextW(hCategory, L"Personal");
                    currentColor = RGB(53, 50, 50);
                    g_lastSelectedEntryIndex = -1;
                    g_dataModifiedInFields = false;
                    ListView_SetItemState(hListView, -1, 0, LVIS_SELECTED | LVIS_FOCUSED);
                    UpdatePasswordStrength(hWnd, hPassword);
                    SetFocus(hWnd);
                }
            }
            else {
                MessageBoxW(hWnd, L"No entry selected.", L"Error", MB_OK | MB_ICONWARNING);
            }
            break;
        }
        case IDC_SEARCH:
        {
            WCHAR searchText[256];
            GetWindowTextW(hSearchEdit, searchText, 256);
            std::wstring search = searchText;
            ListView_DeleteAllItems(hListView);
            if (entries.size() > INT_MAX) {
                MessageBoxW(hWnd, L"Too many entries for ListView.", L"Error", MB_OK | MB_ICONERROR);
                return 0;
            }
            for (size_t i = 0; i < entries.size(); ++i) {
                const PasswordEntry& e = entries[i];
                LVITEMW lvi = { 0 };
                lvi.mask = LVIF_TEXT | LVIF_PARAM;
                lvi.iItem = static_cast<int>(i);
                if (search.empty() ||
                    e.name.find(search) != std::wstring::npos ||
                    e.website.find(search) != std::wstring::npos ||
                    e.email.find(search) != std::wstring::npos ||
                    e.user.find(search) != std::wstring::npos ||
                    e.password.find(search) != std::wstring::npos ||
                    e.note.find(search) != std::wstring::npos ||
                    e.category.find(search) != std::wstring::npos)
                {
                    lvi.pszText = const_cast<LPWSTR>(e.name.c_str());
                    lvi.lParam = (LPARAM)i;
                }
                else {
                    lvi.pszText = (LPWSTR)L"------";
                    lvi.lParam = (LPARAM)-1;
                }
                ListView_InsertItem(hListView, &lvi);
            }
            UpdateListViewColors();
            hHeader = ListView_GetHeader(hListView);
            ShowWindow(hHeader, SW_HIDE);
            SetFocus(hWnd);
            break;
        }
        case IDC_reset:
        {
            ListView_DeleteAllItems(hListView);
            if (entries.size() > INT_MAX) {
                MessageBoxW(hWnd, L"Too many entries for ListView.", L"Error", MB_OK | MB_ICONERROR);
                return 0;
            }
            for (size_t i = 0; i < entries.size(); ++i) {
                const PasswordEntry& e = entries[i];
                LVITEMW lvi = { 0 };
                lvi.mask = LVIF_TEXT | LVIF_PARAM;
                lvi.iItem = static_cast<int>(i);
                lvi.pszText = const_cast<LPWSTR>(e.name.c_str());
                lvi.lParam = (LPARAM)i;
                ListView_InsertItem(hListView, &lvi);
            }
            UpdateListViewColors();
            hHeader = ListView_GetHeader(hListView);
            ShowWindow(hHeader, SW_HIDE);
            ShowScrollBar(hListView, SB_VERT, FALSE);
            ShowScrollBar(hListView, SB_HORZ, FALSE);
            SetFocus(hWnd);
            break;
        }



        case IDC_RESTORE_BACKUP:
        {
            if (MessageBoxW(hWnd, L"Restore from backup? This will overwrite current entries.", L"Confirm Restore", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                std::wstring backupPath = GetFullFilePath(L"DataBackup.xml");
                std::wstring targetPath = GetFullFilePath(L"data.xml");

                // Debug: Verify file paths
                WCHAR debugMsg[512];
                swprintf_s(debugMsg, L"Backup path: %s\nTarget path: %s", backupPath.c_str(), targetPath.c_str());
                MessageBoxW(hWnd, debugMsg, L"Debug Paths", MB_OK);

                if (PathFileExistsW(backupPath.c_str())) {
                    // Check backup file size
                    WIN32_FILE_ATTRIBUTE_DATA fileInfo;
                    if (GetFileAttributesExW(backupPath.c_str(), GetFileExInfoStandard, &fileInfo) && fileInfo.nFileSizeLow == 0) {
                        MessageBoxW(hWnd, L"Backup file is empty.", L"Error", MB_OK | MB_ICONERROR);
                        break;
                    }

                    // Copy backup file
                    if (CopyFileW(backupPath.c_str(), targetPath.c_str(), FALSE)) {
                        // Load XML and populate ListView
                        entries.clear();
                        // Use g_enteredPassword or re-prompt for password
                        INT_PTR result = DialogBoxW(hInst, MAKEINTRESOURCEW(IDD_PASSWORD_DIALOG), hWnd, PasswordDialogProcNew);
                        if (result == IDOK && !g_enteredPassword.empty()) {
                            if (LoadXML(targetPath)) {
                                PopulateListView();
                                ClearSensitiveDataAndUI(hWnd);
                                MessageBoxW(hWnd, L"Backup restored successfully.", L"Success", MB_OK | MB_ICONINFORMATION);
                            }
                            else {
                                swprintf_s(debugMsg, L"Failed to load %s. Check file content or password.", targetPath.c_str());
                                MessageBoxW(hWnd, debugMsg, L"Load Error", MB_OK | MB_ICONERROR);
                            }
                            SecureZeroMemory(&g_enteredPassword[0], g_enteredPassword.size() * sizeof(wchar_t));
                            g_enteredPassword.clear();
                        }
                        else {
                            MessageBoxW(hWnd, L"Password entry cancelled or empty.", L"Error", MB_OK | MB_ICONERROR);
                        }
                    }
                    else {
                        DWORD err = GetLastError();
                        swprintf_s(debugMsg, L"Copy failed. Error code: %d", err);
                        MessageBoxW(hWnd, debugMsg, L"Copy Error", MB_OK | MB_ICONERROR);
                    }
                }
                else {
                    MessageBoxW(hWnd, L"Backup file not found.", L"Error", MB_OK | MB_ICONERROR);
                }
            }
            break;
        }


        case IDC_TOGGLE_THEME:
        {
            isDarkTheme = !isDarkTheme;
            dark = isDarkTheme ? RGB(33, 33, 33) : RGB(22, 22, 22);
            textColor = isDarkTheme ? RGB(255, 255, 255) : RGB(222, 222,8 ); 
            DeleteObject(hDarkGreyBrush);
            hDarkGreyBrush = CreateSolidBrush(dark);
            ListView_SetBkColor(hListView, dark);
            ListView_SetTextBkColor(hListView, dark);
            ListView_SetTextColor(hListView, textColor);
            InvalidateRect(hWnd, nullptr, TRUE);
            UpdateWindow(hWnd);
            break;
        }
        case IDC_COLOR:
        {
            cc.rgbResult = currentColor;
            cc.Flags = CC_RGBINIT | CC_SOLIDCOLOR;
            if (ChooseColor(&cc)) {
                currentColor = cc.rgbResult;
                int idx = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
                if (idx >= 0 && static_cast<size_t>(idx) < entries.size()) {
                    entries[idx].color = currentColor;
                    SaveXML();
                    UpdateListViewColors();
                }
                InvalidateRect(hColorBtn, nullptr, TRUE);
                ShowWindow(hHeader, SW_HIDE);
            }
            break;
        }
        case IDC_COPY_NAME:
        {
            WCHAR buffer[1024];
            GetWindowTextW(hName, buffer, 1024);
            CopyToClipboard(buffer);
            break;
        }
        case IDC_COPY_WEBSITE:
        {
            WCHAR buffer[1024];
            GetWindowTextW(hWebsite, buffer, 1024);
            CopyToClipboard(buffer);
            break;
        }
        case IDC_COPY_EMAIL:
        {
            WCHAR buffer[1024];
            GetWindowTextW(hEmail, buffer, 1024);
            CopyToClipboard(buffer);
            break;
        }
        case IDC_COPY_USER:
        {
            WCHAR buffer[1024];
            GetWindowTextW(hUser, buffer, 1024);
            CopyToClipboard(buffer);
            break;
        }
        case IDC_COPY_PASSWORD:
        {
            WCHAR buffer[1024];
            GetWindowTextW(hPassword, buffer, 1024);
            CopyToClipboard(buffer);
            break;
        }
        case IDC_COPY_NOTE:
        {
            WCHAR buffer[1024];
            GetWindowTextW(hNote, buffer, 1024);
            CopyToClipboard(buffer);
            break;
        }
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDC_PWRD), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }

    case WM_KEYDOWN:
    {
        if (wParam == VK_ESCAPE)
        {
            SetFocus(hWnd);
            DestroyWindow(hWnd);
        }
        else if (wParam == VK_UP || wParam == VK_DOWN)
        {
            if (GetFocus() == hListView)
            {
                int currIdx = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
                if (currIdx == -1 && entries.size() > 0)
                {
                    currIdx = 0;
                }
                else if (currIdx >= 0)
                {
                    if (g_dataModifiedInFields && g_lastSelectedEntryIndex != -1 &&
                        static_cast<size_t>(g_lastSelectedEntryIndex) < entries.size() &&
                        !g_isInsideApplyChanges)
                    {
                        g_isInsideApplyChanges = true;
                        ApplyEntryChanges(hWnd, g_lastSelectedEntryIndex);
                        g_isInsideApplyChanges = false;
                    }
                }

                int newIdx = currIdx;
                if (wParam == VK_UP && currIdx > 0)
                {
                    newIdx = currIdx - 1;
                }
                else if (wParam == VK_DOWN && currIdx < static_cast<int>(entries.size()) - 1)
                {
                    newIdx = currIdx + 1;
                }

                if (newIdx >= 0 && static_cast<size_t>(newIdx) < entries.size())
                {
                    ListView_SetItemState(hListView, currIdx, 0, LVIS_SELECTED | LVIS_FOCUSED);
                    ListView_SetItemState(hListView, newIdx, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                    ListView_EnsureVisible(hListView, newIdx, FALSE);
                    SetWindowTextW(hName, entries[newIdx].name.c_str());
                    SetWindowTextW(hWebsite, entries[newIdx].website.c_str());
                    SetWindowTextW(hEmail, entries[newIdx].email.c_str());
                    SetWindowTextW(hUser, entries[newIdx].user.c_str());
                    SetWindowTextW(hPassword, entries[newIdx].password.c_str());
                    SetWindowTextW(hNote, entries[newIdx].note.c_str());
                    SetWindowTextW(hCategory, entries[newIdx].category.c_str());
                    currentColor = entries[newIdx].color;
                    g_lastSelectedEntryIndex = newIdx;
                    g_dataModifiedInFields = false;
                    UpdatePasswordStrength(hWnd, hPassword);
                }
                return TRUE;
            }
        }
        ShowScrollBar(hListView, SB_VERT, FALSE);
        ShowScrollBar(hListView, SB_HORZ, FALSE);
        hHeader = ListView_GetHeader(hListView);
        ShowWindow(hHeader, SW_HIDE);
        return 0;
    }

    case WM_SETCURSOR:
    {
        if (hCustomCursor) {
            SetCursor(hCustomCursor);
            return TRUE;
        }
        break;
    }

    case WM_CLOSE:
    {
        PasswordEntry entry;
        WCHAR buffer[1024];
        GetWindowTextW(hName, buffer, 1024);
        entry.name = buffer;
        if (!entry.name.empty()) {
            GetWindowTextW(hWebsite, buffer, 1024);
            entry.website = buffer;
            GetWindowTextW(hEmail, buffer, 1024);
            entry.email = buffer;
            GetWindowTextW(hUser, buffer, 1024);
            entry.user = buffer;
            GetWindowTextW(hPassword, buffer, 1024);
            entry.password = buffer;
            GetWindowTextW(hNote, buffer, 1024);
            entry.note = buffer;
            GetWindowTextW(hCategory, buffer, 1024);
            entry.category = buffer;
            entry.color = currentColor;
            bool updated = false;
            bool exists = false;
            for (auto& e : entries) {
                if (_wcsicmp(e.name.c_str(), entry.name.c_str()) == 0) {
                    exists = true;
                    if (e.website != entry.website ||
                        e.email != entry.email ||
                        e.user != entry.user ||
                        e.password != entry.password ||
                        e.note != entry.note ||
                        e.category != entry.category ||
                        e.color != entry.color)
                    {
                        e = entry;
                        updated = true;
                    }
                    break;
                }
            }
            if (!exists) {
                entries.push_back(entry);
                updated = true;
            }
            if (updated) {
                SaveXML();
            }
        }
        KillTrayIcon();
        DestroyWindow(hWnd);
        return 0;
    }

    case WM_DESTROY:
    {
        KillTimer(hWnd, ONE);
        DeleteObject(hBigFont);
        KillTrayIcon();
        if (hBitmap)
            DeleteObject(hBitmap);
        if (hCustomCursor)
            DeleteObject(hCustomCursor);
        if (hDarkGreyBrush)
            DeleteObject(hDarkGreyBrush);
        if (butBrush)
            DeleteObject(butBrush);
        if (hToggleStartupBtn) DestroyWindow(hToggleStartupBtn);
        ShutdownGDIPlus();
        CoUninitialize();
        PostQuitMessage(0);
        return 0;
    }

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

std::wstring Utf8ToWstring(const char* utf8) {
    if (!utf8) return L"";
    int wideLen = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, nullptr, 0);
    if (wideLen == 0) return L"";
    std::vector<wchar_t> buffer(wideLen);
    MultiByteToWideChar(CP_UTF8, 0, utf8, -1, buffer.data(), wideLen);
    return std::wstring(buffer.data());
}

 
tinyxml2::XMLDocument tiny;
 
 

bool LoadXML(std::wstring xmlPath) {

    WCHAR debugMsg[512];

    tiny.Clear();

    // First check if file exists
    if (!PathFileExistsW(xmlPath.c_str())) {
        // Create empty XML if file doesn't exist
        tinyxml2::XMLDocument doc;
        doc.Clear();
        doc.InsertFirstChild(doc.NewDeclaration());
        tinyxml2::XMLElement* root = doc.NewElement("Passwords");
        doc.InsertEndChild(root);

        std::string xmlPathUtf8 = WstringToUtf8(xmlPath);
        if (doc.SaveFile(xmlPathUtf8.c_str()) != tinyxml2::XML_SUCCESS) {
            return false;
        }
        entries.clear();
        return true;
    }

    // Decrypt to temp file
    std::wstring tempPath = GetFullFilePath(L"temp_decrypted.xml");
    if (!DecryptFile(xmlPath, tempPath, std::wstring(g_userKeyForXml.begin(), g_userKeyForXml.end()))) {
        DeleteFileW(tempPath.c_str());
        return false;
    }

    // Load decrypted XML
    tinyxml2::XMLDocument doc;
    doc.Clear();
    std::string tempPathUtf8 = WstringToUtf8(tempPath);
    if (doc.LoadFile(tempPathUtf8.c_str()) != tinyxml2::XML_SUCCESS) {
        DeleteFileW(tempPath.c_str());
        return false;
    }

    entries.clear();

    // Parse XML and populate entries
    tinyxml2::XMLElement* root = doc.FirstChildElement("Passwords");
    if (!root) {
        swprintf_s(debugMsg, L"No 'Passwords' root element found in %s.", tempPath.c_str());
        MessageBoxW(nullptr, debugMsg, L"Error", MB_OK | MB_ICONERROR);
        DeleteFileW(tempPath.c_str());
        return false;
    }

    for (tinyxml2::XMLElement* entryElement = root->FirstChildElement("Entry"); entryElement; entryElement = entryElement->NextSiblingElement("Entry")) {
        PasswordEntry entry;
        const char* name = entryElement->FirstChildElement("Name") ? entryElement->FirstChildElement("Name")->GetText() : "";
        entry.name = Utf8ToWstring(name);
        const char* website = entryElement->FirstChildElement("Website") ? entryElement->FirstChildElement("Website")->GetText() : "";
        entry.website = Utf8ToWstring(website);
        const char* email = entryElement->FirstChildElement("Email") ? entryElement->FirstChildElement("Email")->GetText() : "";
        entry.email = Utf8ToWstring(email);
        const char* user = entryElement->FirstChildElement("User") ? entryElement->FirstChildElement("User")->GetText() : "";
        entry.user = Utf8ToWstring(user);
        const char* password = entryElement->FirstChildElement("Password") ? entryElement->FirstChildElement("Password")->GetText() : "";
        entry.password = Utf8ToWstring(password);
        const char* note = entryElement->FirstChildElement("Note") ? entryElement->FirstChildElement("Note")->GetText() : "";
        entry.note = Utf8ToWstring(note);
        const char* category = entryElement->FirstChildElement("Category") ? entryElement->FirstChildElement("Category")->GetText() : "";
        entry.category = Utf8ToWstring(category);
        tinyxml2::XMLElement* colorElement = entryElement->FirstChildElement("Color");
        if (colorElement) {
            int r, g, b;
            colorElement->QueryIntAttribute("R", &r);
            colorElement->QueryIntAttribute("G", &g);
            colorElement->QueryIntAttribute("B", &b);
            entry.color = RGB(r, g, b);
        }
        else {
            entry.color = RGB(255, 255, 255); // Default color if none specified
        }
        entries.push_back(entry);
    }

    // Clean up temp file
    if (!DeleteFileW(tempPath.c_str())) {
        swprintf_s(debugMsg, L"Failed to delete temp file %s. Error code: %lu", tempPath.c_str(), GetLastError());
        MessageBoxW(nullptr, debugMsg, L"Cleanup Error", MB_OK | MB_ICONWARNING);
    }

    swprintf_s(debugMsg, L"Loaded %zu entries from XML.", entries.size());
    // MessageBoxW(nullptr, debugMsg, L"Debug", MB_OK);
    PopulateListView();
    DeleteFileW(tempPath.c_str());
    return true;
}
 

 





void SaveXML() {
    std::wstring xmlPath = GetFullFilePath(L"data.xml");
    std::wstring tempPath = GetFullFilePath(L"temp_plain.xml");
    std::wstring backupPath = GetFullFilePath(L"DataBackup.xml");

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLDeclaration* decl = doc.NewDeclaration(); // XML declaration (e.g., <?xml version="1.0" encoding="UTF-8"?>)
    doc.InsertFirstChild(decl);

    tinyxml2::XMLElement* root = doc.NewElement("Passwords"); // Root element
    doc.InsertEndChild(root);

    for (const auto& entry : entries) {
        tinyxml2::XMLElement* entryElement = doc.NewElement("Entry");

        // Helper lambda to create and append text elements
        auto createTextElement = [&](const char* elementName, const std::wstring& text) {
            tinyxml2::XMLElement* elem = doc.NewElement(elementName);
            // WstringToUtf8 from CryptoUtils.cpp handles empty wstr by returning empty std::string.
            // tinyxml2 SetText correctly handles empty strings.
            elem->SetText(WstringToUtf8(text).c_str());
            entryElement->InsertEndChild(elem);
            };

        createTextElement("Name", entry.name);
        createTextElement("Website", entry.website);
        createTextElement("Email", entry.email);
        createTextElement("User", entry.user);
        createTextElement("Password", entry.password);
        createTextElement("Note", entry.note);
        createTextElement("Category", entry.category);

        tinyxml2::XMLElement* colorElement = doc.NewElement("Color");
        colorElement->SetAttribute("R", GetRValue(entry.color));
        colorElement->SetAttribute("G", GetGValue(entry.color));
        colorElement->SetAttribute("B", GetBValue(entry.color));
        entryElement->InsertEndChild(colorElement);

        root->InsertEndChild(entryElement);
    }

    // Save to temp plain file first
    std::string tempPathUtf8 = WstringToUtf8(tempPath);
    if (doc.SaveFile(tempPathUtf8.c_str()) != tinyxml2::XML_SUCCESS) {
        MessageBoxW(nullptr, L"Failed to save temporary XML data. Data not saved.", L"Save Error", MB_OK | MB_ICONERROR);
        // Attempt to delete the potentially corrupted/incomplete temp file
        DeleteFileW(tempPath.c_str());
        return;
    }

    // Create a backup of the current encrypted data.xml before overwriting
    if (PathFileExistsW(xmlPath.c_str())) {
        if (!CopyFileW(xmlPath.c_str(), backupPath.c_str(), FALSE)) {
            // Non-critical error, perhaps just log or inform user without stopping the save
            MessageBoxW(nullptr, L"Warning: Failed to create backup of existing data file. Proceeding with save.", L"Backup Warning", MB_OK | MB_ICONWARNING);
        }
    }

    // Encrypt temp file to the final data.xml
    if (!g_userKeyForXml.empty()) { // Ensure key is available
        if (!EncryptFile(tempPath, xmlPath, std::wstring(g_userKeyForXml.begin(), g_userKeyForXml.end()))) {
            MessageBoxW(nullptr, L"Failed to encrypt XML data. Data not saved.", L"Encryption Error", MB_OK | MB_ICONERROR);
            // Clean up temp plain file if encryption fails
            DeleteFileW(tempPath.c_str());
            return;
        }
    }
    else {
        MessageBoxW(nullptr, L"User key not available for encryption. Data not saved.", L"Key Error", MB_OK | MB_ICONERROR);
        // Clean up temp plain file if key is not available
        DeleteFileW(tempPath.c_str());
        return;
    }

    // Clean up temp plain file only if everything was successful
    DeleteFileW(tempPath.c_str());
}



void CopyToClipboard(const std::wstring& text) {
    if (OpenClipboard(nullptr)) {
        EmptyClipboard();
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (text.length() + 1) * sizeof(wchar_t));
        if (hMem) {
            wchar_t* pMem = (wchar_t*)GlobalLock(hMem);
            wcscpy_s(pMem, text.length() + 1, text.c_str());
            GlobalUnlock(hMem);
            SetClipboardData(CF_UNICODETEXT, hMem);
        }
        CloseClipboard();
    }
}

void UpdateListViewColors() {
    InvalidateRect(hListView, nullptr, TRUE);
    UpdateWindow(hListView);
}

void PopulateListView() {
    ListView_DeleteAllItems(hListView);
    for (size_t i = 0; i < entries.size(); ++i) {
        LVITEMW lvi = { 0 };
        lvi.mask = LVIF_TEXT;
        lvi.iItem = (int)i;
        lvi.pszText = (LPWSTR)entries[i].name.c_str();
        ListView_InsertItem(hListView, &lvi);
    }
   // WCHAR debugMsg[256];
   // swprintf_s(debugMsg, L"Populated %d items", ListView_GetItemCount(hListView));
   // MessageBoxW(nullptr, debugMsg, L"PopulateListView Debug", MB_OK);
}

 



// Dialog procedure for the new password dialog
INT_PTR CALLBACK PasswordDialogProcNew(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    static HBRUSH hEditBrush = NULL; // Static brush for edit control background

    switch (message)
    {
    case WM_INITDIALOG:
    {
        if (isDarkTheme && hEditBrush == NULL) {
            hEditBrush = CreateSolidBrush(RGB(50, 50, 50));
        }
        std::wstring pgpPath = GetFullFilePath(L"password.pgp");
        g_isInVerifyModeNewDialog = PathFileExistsW(pgpPath.c_str());
        SetWindowTextW(hDlg, g_isInVerifyModeNewDialog ? L"Verify Password" : L"Create New Password");
        HWND hwndParent = GetParent(hDlg) ? GetParent(hDlg) : GetDesktopWindow();
        RECT rcDlg, rcParent;
        GetWindowRect(hDlg, &rcDlg);
        GetWindowRect(hwndParent, &rcParent);
        int nWidth = rcDlg.right - rcDlg.left;
        int nHeight = rcDlg.bottom - rcDlg.top;
        int x = rcParent.left + (rcParent.right - rcParent.left - nWidth) / 2;
        int y = rcParent.top + (rcParent.bottom - rcParent.top - nHeight) / 2;
        SetWindowPos(hDlg, NULL, x, y, nWidth, nHeight, SWP_NOZORDER | SWP_NOACTIVATE);
        HWND hPasswordEdit = GetDlgItem(hDlg, IDC_PASSWORD_EDIT);
        if (hPasswordEdit) {
            SetFocus(hPasswordEdit);
        }
        else {
            MessageBoxW(hDlg, L"Password edit control not found.", L"Error", MB_OK | MB_ICONERROR);
        }
        return (INT_PTR)FALSE;
    }

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDOK:
        {
            HWND hEdit = GetDlgItem(hDlg, IDC_PASSWORD_EDIT);
            WCHAR passwordBuffer[256];
            GetWindowTextW(hEdit, passwordBuffer, ARRAYSIZE(passwordBuffer));
            g_enteredPassword = passwordBuffer;
            SecureZeroMemory(passwordBuffer, sizeof(passwordBuffer));
            EndDialog(hDlg, IDOK);
            return (INT_PTR)TRUE;
        }
        case IDCANCEL:
            g_enteredPassword.clear(); // Clear password on cancel
            EndDialog(hDlg, IDCANCEL);
            return (INT_PTR)TRUE;
        }
        break;
    }

    case WM_CTLCOLORDLG:
        if (isDarkTheme) {
            // Dialog background
            return (INT_PTR)hDarkGreyBrush;
        }
        // If not dark theme, let DefWindowProc handle it or return a light theme brush
        break;

    case WM_CTLCOLORSTATIC:
    {
        HDC hdcStatic = (HDC)wParam;
        if (isDarkTheme) {
            SetTextColor(hdcStatic, textColor); // Global dark theme text color
            SetBkMode(hdcStatic, TRANSPARENT);
            return (INT_PTR)hDarkGreyBrush; // Background for static control (same as dialog)
        }
        break;
    }

    case WM_CTLCOLOREDIT:
    {
        HDC hdcEdit = (HDC)wParam;
        if (isDarkTheme) {
            SetTextColor(hdcEdit, textColor); // Global dark theme text color for edit control text
            SetBkMode(hdcEdit, TRANSPARENT);
            // Use the specific brush for edit control background if initialized
            if (hEditBrush != NULL) {
                return (INT_PTR)hEditBrush;
            }
            else {
                // Fallback if brush wasn't created, though it should be in WM_INITDIALOG
                return (INT_PTR)hDarkGreyBrush;
            }
        }
        break;
    }

    case WM_CTLCOLORBTN:
    {
        // This message is sent for standard buttons (not BS_OWNERDRAW).
        // For dark theme, we want the button text to be light and the background to match the dialog.
        // However, standard buttons are notoriously difficult to color perfectly without owner draw.
        // This will attempt to set text color and background.
        HDC hdcButton = (HDC)wParam;
        if (isDarkTheme) {
            SetTextColor(hdcButton, textColor); // Light text
            SetBkMode(hdcButton, TRANSPARENT); // Try to make standard button background transparent
            // so hDarkGreyBrush (dialog bg) shows through.
            // This has limited effect on standard button appearance.
            return (INT_PTR)hDarkGreyBrush;
        }
        break;
    }
    case WM_DESTROY:
        // Clean up static brush used for edit control background
        if (hEditBrush != NULL) {
            DeleteObject(hEditBrush);
            hEditBrush = NULL;
        }
        // If any other GDI objects were created per-dialog instance, clean them here.
        break;

    }
    return (INT_PTR)FALSE; // Default processing for other messages
}



 



int cEXIST(HWND hWnd)
{

    std::wstring tempPassword = L"Password"; // Declare and initialize tempPassword
    SecureZeroMemory(&tempPassword[0], tempPassword.size() * sizeof(wchar_t));

    std::wstring pgpPath = GetFullFilePath(L"password.pgp");
    bool fileExists = PathFileExistsW(pgpPath.c_str());

    while (true)
    {
        g_isInVerifyModeNewDialog = fileExists;
        g_enteredPassword.clear();
        INT_PTR result = DialogBoxW(hInst, MAKEINTRESOURCEW(IDD_PASSWORD_DIALOG), hWnd, PasswordDialogProcNew);

        if (result == IDCANCEL)
        {
            ClearSensitiveDataAndUI(hWnd);
            return 0;
        }
        else if (result == IDOK)
        {
            // Trim whitespace from entered password
            g_enteredPassword.erase(0, g_enteredPassword.find_first_not_of(L" \t\n\r"));
            g_enteredPassword.erase(g_enteredPassword.find_last_not_of(L" \t\n\r") + 1);

            if (g_enteredPassword.empty())
            {
                MessageBoxW(hWnd, L"Password cannot be empty. Please enter a valid password.", L"Error", MB_OK | MB_ICONERROR);
                continue;
            }

            // Store the password temporarily for LoadXML
            std::wstring tempPassword = g_enteredPassword;

            if (!fileExists)
            {
                try
                {
                    // Generate key and salt for new password
                    auto [key, salt] = GenerateKeyAndSaltFromPassword(g_enteredPassword);
                    std::ofstream outFile(pgpPath, std::ios::binary);
                    if (outFile.is_open())
                    {
                        // Write salt followed by key to password.pgp
                        outFile.write(reinterpret_cast<const char*>(salt.data()), salt.size());
                        outFile.write(reinterpret_cast<const char*>(key.data()), key.size());
                        outFile.close();
                        g_userKeyForXml = key;
                        g_isPinValidated = true;
                        SecureZeroMemory(&g_enteredPassword[0], g_enteredPassword.size() * sizeof(wchar_t));
                        g_enteredPassword.clear();
                        // Load XML with the original password
                        if (!LoadXML(GetFullFilePath(L"data.xml")))
                        {
                            MessageBoxW(hWnd, L"Failed to initialize data file.", L"Error", MB_OK | MB_ICONERROR);
                            SecureZeroMemory(&tempPassword[0], tempPassword.size() * sizeof(wchar_t));
                            return 0;
                        }
                        SecureZeroMemory(&tempPassword[0], tempPassword.size() * sizeof(wchar_t));
                        tempPassword.clear();
                        return 1;
                    }
                    else
                    {
                        MessageBoxW(hWnd, L"Failed to create password file.", L"Error", MB_OK | MB_ICONERROR);
                        SecureZeroMemory(&tempPassword[0], tempPassword.size() * sizeof(wchar_t));
                        return 0;
                    }
                }
                catch (const std::exception& e)
                {
                    WCHAR debugMsg[512];
                    swprintf_s(debugMsg, L"Error generating key: %S", e.what());
                    MessageBoxW(hWnd, debugMsg, L"Error", MB_OK | MB_ICONERROR);
                    SecureZeroMemory(&tempPassword[0], tempPassword.size() * sizeof(wchar_t));
                    return 0;
                }
            }
            else
            {
                std::ifstream inFile(pgpPath, std::ios::binary);
                if (inFile.is_open())
                {
                    // Read salt and stored key from password.pgp
                    std::vector<BYTE> fileData((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
                    inFile.close();
                    if (fileData.size() < SALT_LENGTH + KEY_LENGTH)
                    {
                        MessageBoxW(hWnd, L"Corrupted password file.", L"Error", MB_OK | MB_ICONERROR);
                        return 0;
                    }
                    std::vector<BYTE> storedSalt(fileData.begin(), fileData.begin() + SALT_LENGTH);
                    std::vector<BYTE> storedKey(fileData.begin() + SALT_LENGTH, fileData.begin() + SALT_LENGTH + KEY_LENGTH);

                    try
                    {
                        // Generate key from entered password using stored salt
                        std::vector<BYTE> enteredKey = GenerateKeyFromPassword(g_enteredPassword, storedSalt);
                        if (enteredKey == storedKey)
                        {
                            g_userKeyForXml = enteredKey;
                            g_isPinValidated = true;
                            SecureZeroMemory(&g_enteredPassword[0], g_enteredPassword.size() * sizeof(wchar_t));
                            g_enteredPassword.clear();
                            // Load XML with the original password
                            if (!LoadXML(GetFullFilePath(L"data.xml")))
                            {
                                MessageBoxW(hWnd, L"Failed to load XML file.", L"Error", MB_OK | MB_ICONERROR);
                                SecureZeroMemory(&tempPassword[0], tempPassword.size() * sizeof(wchar_t));
                                 return 1;////////////////////
                                 ///////////////
                            }
                            SecureZeroMemory(&tempPassword[0], tempPassword.size() * sizeof(wchar_t));
                            tempPassword.clear();
                            return 1;
                        }
                        else
                        {
                            g_pin_attempts++;
                            if (g_pin_attempts >= 3)
                            {
                                MessageBoxW(hWnd, L"Too many incorrect attempts. Application will exit.", L"Error", MB_OK | MB_ICONERROR);
                                ClearSensitiveDataAndUI(hWnd);
                                SecureZeroMemory(&tempPassword[0], tempPassword.size() * sizeof(wchar_t));
                                return 0;
                            }
                            MessageBoxW(hWnd, L"Password is incorrect. Please try again.", L"Error", MB_OK | MB_ICONERROR);
                            SecureZeroMemory(&tempPassword[0], tempPassword.size() * sizeof(wchar_t));
                            continue;
                        }
                    }
                    catch (const std::exception& e)
                    {
                        WCHAR debugMsg[512];
                        swprintf_s(debugMsg, L"Error verifying key: %S", e.what());
                        MessageBoxW(hWnd, debugMsg, L"Error", MB_OK | MB_ICONERROR);
                        SecureZeroMemory(&tempPassword[0], tempPassword.size() * sizeof(wchar_t));
                        return 0;
                    }
                }
                else
                {
                    MessageBoxW(hWnd, L"Failed to read password file.", L"Error", MB_OK | MB_ICONERROR);
                    SecureZeroMemory(&tempPassword[0], tempPassword.size() * sizeof(wchar_t));
                    return 0;
                }
            }
        }
        else
        {
            MessageBoxW(hWnd, L"Unexpected dialog result. Please try again.", L"Error", MB_OK | MB_ICONERROR);
            SecureZeroMemory(&tempPassword[0], tempPassword.size() * sizeof(wchar_t));
           
            continue;
        }
    }
    return 0;
}





// Check if the application is set to run at startup
bool IsStartupEnabled() {
    HKEY hKey;
    bool enabled = false;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, REG_RUN_KEY, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD dataSize = 0;
        if (RegQueryValueExW(hKey, APP_NAME, NULL, NULL, NULL, &dataSize) == ERROR_SUCCESS) {
            enabled = true;
        }
        RegCloseKey(hKey);
    }
    return enabled;
}

// Enable application to run at startup
bool EnableStartup() {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, REG_RUN_KEY, 0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        WCHAR exePath[MAX_PATH];
        GetModuleFileNameW(NULL, exePath, MAX_PATH);
        // Add quotes around the path to handle spaces
        std::wstring quotedPath = L"\"" + std::wstring(exePath) + L"\"";
        LONG result = RegSetValueExW(hKey, APP_NAME, 0, REG_SZ,
            (const BYTE*)quotedPath.c_str(),
            (quotedPath.length() + 1) * sizeof(WCHAR));
        RegCloseKey(hKey);
        return result == ERROR_SUCCESS;
    }
    return false;
}

// Disable application from running at startup
bool DisableStartup() {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, REG_RUN_KEY, 0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        LONG result = RegDeleteValueW(hKey, APP_NAME);
        RegCloseKey(hKey);
        return result == ERROR_SUCCESS || result == ERROR_FILE_NOT_FOUND;
    }
    return false;
}