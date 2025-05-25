// Pwrd.cpp : Defines the entry point for the application.
//

#include "pch.h"
#include "framework.h"
#include "Pwrd.h"

#include <thread>
#include <commctrl.h>
#include <gdiplus.h>
#include <objidl.h>
#include <commdlg.h>
#include <string>
#include <random>
#include <windowsx.h>
#include <windows.h>
 
#include <objbase.h>
 
#include <shlwapi.h>
#include "trayicon.h"

 


#include "tinyxml2.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shlwapi.lib")

#define MAX_LOADSTRING 100
#define ONE 1

// Global Variables:
HINSTANCE hInst = nullptr;                               // current instance
WCHAR szTitle[MAX_LOADSTRING] = { 0 };
WCHAR szWindowClass[MAX_LOADSTRING] = { 0 };
WCHAR exePath[MAX_PATH] = { 0 };                // Changed to WCHAR for Unicode

HBITMAP hBitmap = nullptr;                      // Handle for the loaded bitmap
HCURSOR hCustomCursor = nullptr;
HFONT hBigFont = nullptr;
 
TOOLINFO ti = { 0 };;

ULONG_PTR gdiplusToken = 0;
bool dragging = false;                          // Flag to track dragging state
POINT dragStart = { 0, 0 };                            // Start point for dragging
int mx = 0, my = 0;

HWND hTooltip = nullptr;
HWND hListView = nullptr, hName = nullptr, hWebsite = nullptr, hEmail = nullptr, hUser = nullptr, hPassword = nullptr, hNote = nullptr, hSearchEdit = nullptr;
HWND hAddBtn = nullptr, hDeleteBtn = nullptr, hSearchBtn = nullptr, hColorBtn = nullptr;
HWND hCopyNameBtn = nullptr, hCopyWebsiteBtn = nullptr, hCopyEmailBtn = nullptr, hCopyUserBtn = nullptr, hCopyPasswordBtn = nullptr, hCopyNoteBtn = nullptr;
HWND AutoBtn = nullptr, XBtn = nullptr, MidBtn = nullptr, LowBtn = nullptr, hBtnicon = nullptr;
HWND resetBtn = nullptr;

COLORREF currentColor = RGB(222, 222,8); // Default text color
COLORREF dark = RGB(33, 33, 33); // Default black
static CHOOSECOLOR cc = { 0 };
static COLORREF customColors[16] = { 0 };
HBRUSH hDarkGreyBrush = nullptr;
HBRUSH butBrush = nullptr;
std::vector<PasswordEntry> entries;

const wchar_t* animationSets[] = {
    L"/-\\|\0",
    L"⠋⠙⠹⠸⠼⠽⠾⠿\0",
   // L"▂▃▄▅▆▇█▇▆▅▄▃▂\0",
   // L"▉▊▋▌▍▎▏▎▍▌▋▊▉\0",
   // L"🌑🌒🌓🌔🌕🌖🌗🌘\0",
    //L"*✧✺✹✸✷✶✵✴\0",
    L"←↖↑↗→↘↓↙\0",
    L"█▓▒░▒▓█\0",
    //L"⚫⚪\0",
    L"/-\\|\0",
   // L"💨💥✨💫\0",
    L"⏳⌛\0"
};

 
const wchar_t* animationChars;
int Tlength = 0;
int animationIndex = 0; // Current animation frame

// Forward declarations
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK PasswordDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int callpin(HWND hwnd );

void LoadXML();
void SaveXML();
void PopulateListView();
void CopyToClipboard(const std::wstring& text);
void UpdateListViewColors();

const wchar_t* getRandomAnimationSet() {
    std::srand(std::time(nullptr));  // Seed the random number generator
    int index = std::rand() % (sizeof(animationSets) / sizeof(animationSets[0]));

    animationChars = animationSets[index];  // Assign globally
    Tlength = wcslen(animationChars); // Store the length properly

    return animationChars;
}

void InitializeGDIPlus() {
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
}

void ShutdownGDIPlus() {
    Gdiplus::GdiplusShutdown(gdiplusToken);
}

// Function to make the window transparent
void Transparent(HWND hWnd, int alpha) {
    SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), (BYTE)alpha, LWA_ALPHA);
}


void GenerateAndSetPassword(HWND hEdit, int length) {
    if (length < 8) length = 8; // Enforce a minimum

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
    password.reserve(length); // optimization

    for (int i = 0; i < length; ++i) {
        password += charset[dist(gen)];
    }

    SetWindowTextW(hEdit, password.c_str());
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

    // Use Unicode version for consistency
    GetModuleFileNameW(NULL, exePath, MAX_PATH);


   

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PWRD, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PWRD));
    MSG msg;


     

    // Main message loop
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
   // wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_PWRD); // Fixed: Load the IDC_PWRD menu
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    // Check if icons loaded successfully
    if (!wcex.hIcon || !wcex.hIconSm) {
        MessageBoxW(nullptr, L"Failed to load icons!", L"Error", MB_OK | MB_ICONERROR);
    }

   // return RegisterClassExW(&wcex);

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

    if (!hWnd)
    {
        WCHAR errorMsg[256];
        swprintf_s(errorMsg, L"Failed to create window! Error code: %lu", GetLastError());
        MessageBoxW(nullptr, errorMsg, L"Error", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // Apply transparency (e.g., 90% opacity)
   // Transparent(hWnd, 5);

    

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    
   

    return TRUE;
}


void AddTooltip(HWND hTooltip, HWND hWnd, HWND hControl, LPCWSTR text)
{
    TOOLINFO ti = { sizeof(TOOLINFO) };
    ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    ti.hwnd = hWnd;
    ti.uId = (UINT_PTR)hControl;
    ti.lpszText = (LPWSTR)text; // Cast here
    SendMessage(hTooltip, TTM_ADDTOOL, 0, (LPARAM)&ti);
}


void ini(HWND hWnd)
{


      animationChars = getRandomAnimationSet();


    //WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | WS_VSCROLL
    // Initialize list view
    hListView = CreateWindowEx(0, WC_LISTVIEW, L"",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL  ,
        10, 10, 200, 580, hWnd, (HMENU)IDC_LISTVIEW, hInst, nullptr);

    LVCOLUMNW lvc = { 0 };
    lvc.mask = LVCF_TEXT | LVCF_WIDTH;
    lvc.cx = 200;
    //lvc.pszText = (LPWSTR)L"Name";
    lvc.pszText = (LPWSTR)L""; // empty string
    ListView_InsertColumn(hListView, 0, &lvc);
 
     ListView_SetExtendedListViewStyle(hListView, LVS_EX_FULLROWSELECT);
  // ListView_SetExtendedListViewStyle(hListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
     ShowScrollBar(hListView, SB_VERT, FALSE);


    HWND hHeader = ListView_GetHeader(hListView);
   // SetWindowTheme(hListView, L"", L""); // disables theme (cleans up borders & background)
    ListView_SetBkColor(hListView, dark);
    ListView_SetTextBkColor(hListView, dark);
    ListView_SetTextColor(hListView, RGB(255, 255, 255));
    //ShowWindow(hHeader, SW_HIDE);


    // Create edit controls and labels
    CreateWindow(L"STATIC", L"Name", WS_CHILD | WS_VISIBLE,
        220, 10, 80, 20, hWnd, nullptr, hInst, nullptr);
    hName = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE   | ES_AUTOHSCROLL,
        300, 10, 350, 28, hWnd, (HMENU)IDC_NAME, hInst, nullptr);
   

    CreateWindow(L"STATIC", L"Website", WS_CHILD | WS_VISIBLE,
        220, 40, 80, 20, hWnd, nullptr, hInst, nullptr);
    hWebsite = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE  | ES_AUTOHSCROLL,
        300, 40, 350, 28, hWnd, (HMENU)IDC_WEBSITE, hInst, nullptr);
    

    CreateWindow(L"STATIC", L"Email", WS_CHILD | WS_VISIBLE,
        220, 70, 80, 20, hWnd, nullptr, hInst, nullptr);
    hEmail = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE  | ES_AUTOHSCROLL,
        300, 70, 350, 28, hWnd, (HMENU)IDC_EMAIL, hInst, nullptr);
   

    CreateWindow(L"STATIC", L"User", WS_CHILD | WS_VISIBLE,
        220, 100, 80, 20, hWnd, nullptr, hInst, nullptr);
    hUser = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE  | ES_AUTOHSCROLL,
        300, 100, 350, 28, hWnd, (HMENU)IDC_USER, hInst, nullptr);
    
/////////////////////////
     CreateWindow(L"STATIC", L"Password", WS_CHILD | WS_VISIBLE,
         220, 130, 80, 20, hWnd, nullptr, hInst, nullptr);

     

    AutoBtn = CreateWindow(L"BUTTON", L"Auto", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        230, 147, 40, 16, hWnd, (HMENU)IDC_AutoBtn, hInst, nullptr);

    hPassword = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE  | ES_MULTILINE | ES_AUTOVSCROLL,
        300, 130, 350, 28, hWnd, (HMENU)IDC_PASSWORD, hInst, nullptr);

    //hPassword = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_PASSWORD,
        //300, 130, 350, 30, hWnd, (HMENU)IDC_PASSWORD, hInst, nullptr);
////////////////////


    CreateWindow(L"STATIC", L"Note", WS_CHILD | WS_VISIBLE,
        220, 218, 80, 20, hWnd, nullptr, hInst, nullptr);

    hNote = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE   | ES_MULTILINE |   ES_AUTOVSCROLL | ES_AUTOHSCROLL, 
        300, 162, 350, 130, hWnd, (HMENU)IDC_NOTE, hInst, nullptr);



 
///////////////////////////////
    hCopyNameBtn = CreateWindow(L"BUTTON", L"Copy", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        654, 10, 50, 20, hWnd, (HMENU)IDC_COPY_NAME, hInst, nullptr);
    hCopyWebsiteBtn = CreateWindow(L"BUTTON", L"Copy", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        654, 40, 50, 20, hWnd, (HMENU)IDC_COPY_WEBSITE, hInst, nullptr);
    hCopyEmailBtn = CreateWindow(L"BUTTON", L"Copy", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        654, 70, 50, 20, hWnd, (HMENU)IDC_COPY_EMAIL, hInst, nullptr);
    hCopyUserBtn = CreateWindow(L"BUTTON", L"Copy", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        654, 100, 50, 20, hWnd, (HMENU)IDC_COPY_USER, hInst, nullptr);
    hCopyPasswordBtn = CreateWindow(L"BUTTON", L"Copy", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        654, 130, 50, 20, hWnd, (HMENU)IDC_COPY_PASSWORD, hInst, nullptr);

    hCopyNoteBtn = CreateWindow(L"BUTTON", L"Copy", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        654, 180, 50, 20, hWnd, (HMENU)IDC_COPY_NOTE, hInst, nullptr);



    CreateWindow(L"STATIC", L"Color:", WS_CHILD | WS_VISIBLE,
        220, 390, 50, 20, hWnd, nullptr, hInst, nullptr);
    hColorBtn = CreateWindow(L"BUTTON", L"Pick Color", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        270, 390, 80, 20, hWnd, (HMENU)IDC_COLOR, hInst, nullptr);
   


    hAddBtn = CreateWindow(L"BUTTON", L"Add", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        654, 310, 80, 30, hWnd, (HMENU)IDC_ADD, hInst, nullptr);

    hDeleteBtn = CreateWindow(L"BUTTON", L"<-Delete", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        220, 310, 80, 30, hWnd, (HMENU)IDC_DELETE, hInst, nullptr);
/////////////////////////////////////////////


  

 

 


    // Create search controls///////////////////////////////////////////////////////////////////////////

    hSearchBtn = CreateWindow(L"BUTTON", L"Search", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        474, 350, 80, 20, hWnd, (HMENU)IDC_SEARCH, hInst, nullptr);
    //CreateWindow(L"STATIC", L"Search:", WS_CHILD | WS_VISIBLE,
       // 220, 350, 50, 20, hWnd, nullptr, hInst, nullptr);

    resetBtn = CreateWindow(L"BUTTON", L"Search", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        560, 350, 60, 20, hWnd, (HMENU)IDC_reset, hInst, nullptr);

    hSearchEdit = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE  | ES_AUTOHSCROLL,
        270, 350, 200, 20, hWnd, (HMENU)IDC_SEARCH_EDIT, hInst, nullptr);
    ShowScrollBar(hNote, SB_BOTH, FALSE);



    XBtn = CreateWindow(L"BUTTON", L"X", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        774, 5, 22, 30, hWnd, (HMENU)IDC_XBtn, hInst, nullptr);
    MidBtn = CreateWindow(L"BUTTON", L"O", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        774, 35, 22, 30, hWnd, (HMENU)IDC_MidBtn, hInst, nullptr);
    LowBtn = CreateWindow(L"BUTTON", L"-", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        774, 65, 22, 30, hWnd, (HMENU)IDC_LowBtn, hInst, nullptr);




    HICON tmphIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_PWRD));

    // Get the icon info
    ICONINFO iconInfo;
    GetIconInfo(tmphIcon, &iconInfo);

    // Create the button with the correct size
    hBtnicon = CreateWindowEx(
        0, L"BUTTON", nullptr,
        WS_CHILD | WS_VISIBLE | BS_ICON,
        760, 560, iconInfo.xHotspot * 2, iconInfo.yHotspot * 2, // x, y, width, height
        hWnd, (HMENU)1001, hInst, nullptr
    );

    // Set the icon
    SendMessage(hBtnicon, BM_SETIMAGE, IMAGE_ICON, (LPARAM)tmphIcon);;



    /////////////////////////////////////////////////////////////////

    // Create Tooltip Control
        hTooltip = CreateWindowEx(0, TOOLTIPS_CLASS, nullptr,
            WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON, // Use TTS_BALLOON for a modern look
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            hWnd, nullptr, hInst, nullptr);

      
  if (hTooltip)
{
    SetWindowPos(hTooltip, HWND_TOPMOST, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    AddTooltip(hTooltip, hWnd, hName, L"Enter the name of the entry");
    AddTooltip(hTooltip, hWnd, hWebsite, L"Enter the website URL");
    AddTooltip(hTooltip, hWnd, hEmail, L"Enter the email address");
    AddTooltip(hTooltip, hWnd, hUser, L"Enter the username");
    AddTooltip(hTooltip, hWnd, hPassword, L"Enter the password");
    AddTooltip(hTooltip, hWnd, hNote, L"Enter additional notes");
    AddTooltip(hTooltip, hWnd, hSearchEdit, L"Enter search term");
    AddTooltip(hTooltip, hWnd, hAddBtn, L"Add a new entry");
    AddTooltip(hTooltip, hWnd, hDeleteBtn, L"Delete the selected entry");
    AddTooltip(hTooltip, hWnd, hSearchBtn, L"Search entries by name");
    AddTooltip(hTooltip, hWnd, hColorBtn, L"Pick a color for the entry");
    AddTooltip(hTooltip, hWnd, hCopyNameBtn, L"Copy name to clipboard");
    AddTooltip(hTooltip, hWnd, hCopyWebsiteBtn, L"Copy website to clipboard");
    AddTooltip(hTooltip, hWnd, hCopyEmailBtn, L"Copy email to clipboard");
    AddTooltip(hTooltip, hWnd, hCopyUserBtn, L"Copy username to clipboard");
    AddTooltip(hTooltip, hWnd, hCopyPasswordBtn, L"Copy password to clipboard");
    AddTooltip(hTooltip, hWnd, hCopyNoteBtn, L"Copy note to clipboard");
    AddTooltip(hTooltip, hWnd, AutoBtn, L"Generate a random password");

    SendMessage(hTooltip, TTM_UPDATE, 0, 0);
    SendMessage(hTooltip, TTM_TRACKACTIVATE, TRUE, (LPARAM)&ti);
    SendMessage(hTooltip, TTM_SETMAXTIPWIDTH, 0, 100); 
    SendMessage(hTooltip, TTM_SETDELAYTIME, TTDT_AUTOPOP, 1000);

    SendMessageW(hTooltip, TTM_UPDATE, 0, 0);
    SendMessageW(hTooltip, TTM_TRACKACTIVATE, TRUE, (LPARAM)&ti);
    SendMessageW(hTooltip, TTM_SETMAXTIPWIDTH, 0, 100);
    SendMessageW(hTooltip, TTM_SETDELAYTIME, TTDT_AUTOPOP, 1000);
} 



    //////////////////////////////////////////////////////////////



    NONCLIENTMETRICS ncm = { sizeof(NONCLIENTMETRICS) };
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);

    // Modify the font size
    ncm.lfMessageFont.lfHeight = 20; // Bigger size (default is around -11 or -12)

    // Create a new font based on the system font
    HFONT hFont = CreateFontIndirect(&ncm.lfMessageFont);

    // Set it on the edit control
    
    SendMessage(hName, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hWebsite, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hEmail, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hUser, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hPassword, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hNote, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hSearchEdit, WM_SETFONT, (WPARAM)hFont, TRUE);


    ncm.lfMessageFont.lfHeight = 24;
    HFONT hFont2 = CreateFontIndirect(&ncm.lfMessageFont);
    SendMessage(hListView,WM_SETFONT, (WPARAM)hFont2, TRUE);


    // Initialize color picker
    ZeroMemory(&cc, sizeof(cc));
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = hWnd;
    cc.lpCustColors = customColors;
    cc.Flags = CC_FULLOPEN | CC_RGBINIT;

 
    
    ncm.lfMessageFont.lfHeight = 40;
    hBigFont = CreateFontIndirect(&ncm.lfMessageFont);


    SetTimer(hWnd, ONE, 333, nullptr);
    // Load XML data
    LoadXML();
    PopulateListView();

}


/// <summary>
/// ////////////////////////////////
 
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{ 

    HWND hHeader=nullptr;
    static HINSTANCE hInstance = NULL;

    switch (message)
    {
    case WM_CREATE:
    {
        InitCommonControls();
        CoInitialize(NULL);
        OleInitialize(NULL);
        InitializeGDIPlus();


        int result = callpin(hWnd);

        // Create dark grey brush
        hDarkGreyBrush = CreateSolidBrush(dark); //button color
        butBrush = CreateSolidBrush(RGB(66, 66, 66)); //button color

        // Load the bitmap resource
        hBitmap = LoadBitmap(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDB_BITMAP1));
        if (!hBitmap) {
            MessageBoxW(hWnd, L"Failed to load bitmap!", L"Error", MB_OK | MB_ICONERROR);
        }

        // Load the custom cursor
        hCustomCursor = LoadCursor(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDC_CURSOR1));
        if (!hCustomCursor) {
            MessageBoxW(hWnd, L"Failed to load cursor!", L"Error", MB_OK | MB_ICONERROR);
        }

        ini(hWnd);
        ShowScrollBar(hListView, SB_VERT, FALSE);   // Hide vertical scrollbar
        ShowScrollBar(hListView, SB_HORZ, FALSE);   // Hide horizontal scrollbar

         hHeader = ListView_GetHeader(hListView);


          
         
        // CreateTrayIcon(hWnd );
         hInstance = ((LPCREATESTRUCT)lParam)->hInstance;
         CreateTrayIcon(hWnd, hInstance, IDI_PWRD);
            
          

        return 0;
    }

    case WM_TRAYICON: {
        // Handle tray icon messages
        if (lParam == WM_LBUTTONUP || lParam == WM_LBUTTONDBLCLK)
        {
            // Left click or double-click: restore the window
            ShowWindow(hWnd, SW_SHOW);
            ShowWindow(hWnd, SW_RESTORE);
            SetForegroundWindow(hWnd);
            return 0;
        }
        else if (lParam == WM_RBUTTONUP)
        {
            // Right click: show context menu
            POINT pt;
            GetCursorPos(&pt);
            SetForegroundWindow(hWnd);

            // Create popup menu
            HMENU hMenu = CreatePopupMenu();
            if (hMenu)
            {
                // Add menu items
                AppendMenu(hMenu, MF_STRING, 1, _T("Restore"));
                AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
                AppendMenu(hMenu, MF_STRING, 2, _T("Exit"));

                // Track the popup menu
                int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY | TPM_RIGHTBUTTON,
                    pt.x, pt.y, 0, hWnd, NULL);

                // Handle menu commands
                switch (cmd)
                {
                case 1: // Restore
                    ShowWindow(hWnd, SW_SHOW);
                    ShowWindow(hWnd, SW_RESTORE);

                    // Bring window to foreground
                    SetForegroundWindow(hWnd);
                    break;

                case 2: // Exit
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
        

        HWND hHeader = ListView_GetHeader(hListView);
        if (hHeader)
            ShowWindow(hHeader, SW_HIDE);
        break;
    }
  

    case WM_DRAWITEM:
    {
        LPDRAWITEMSTRUCT lpDraw = (LPDRAWITEMSTRUCT)lParam;
        COLORREF Ti = RGB(177, 177, 177); ////////////////////////////////////////////// 
        switch (lpDraw->CtlID)
        {

        case  IDC_reset: {

            HDC hdc = lpDraw->hDC;

  
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, Ti);
            FillRect(hdc, &lpDraw->rcItem, hDarkGreyBrush);
            FrameRect(hdc, &lpDraw->rcItem, (HBRUSH)GetStockObject(BLACK_BRUSH));
            const wchar_t* buttonText = L"Reset";
            DrawText(hdc, buttonText, -1, &lpDraw->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            hHeader = ListView_GetHeader(hListView);
            ShowWindow(hHeader, SW_HIDE);
            return TRUE;
        }



        case  IDC_XBtn: {

            HDC hdc = lpDraw->hDC;

            // Set background color and text color
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(222,222,18));
            FillRect(hdc, &lpDraw->rcItem, hDarkGreyBrush);
            FrameRect(hdc, &lpDraw->rcItem, (HBRUSH)GetStockObject(BLACK_BRUSH));
            const wchar_t* buttonText = L"X";
            DrawText(hdc, buttonText, -1, &lpDraw->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            hHeader = ListView_GetHeader(hListView);
            ShowWindow(hHeader, SW_HIDE);
            return TRUE;
        }

        case IDC_MidBtn: {

            HDC hdc = lpDraw->hDC;

            // Set background color and text color
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(222, 222, 18));
            FillRect(hdc, &lpDraw->rcItem, hDarkGreyBrush);
            FrameRect(hdc, &lpDraw->rcItem, (HBRUSH)GetStockObject(BLACK_BRUSH));
            const wchar_t* buttonText = L"o";
            DrawText(hdc, buttonText, -1, &lpDraw->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            hHeader = ListView_GetHeader(hListView);
            ShowWindow(hHeader, SW_HIDE);
            return TRUE;
        }

        case IDC_LowBtn: {

            HDC hdc = lpDraw->hDC;

            // Set background color and text color
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(222, 222, 18));
            FillRect(hdc, &lpDraw->rcItem, hDarkGreyBrush);
            FrameRect(hdc, &lpDraw->rcItem, (HBRUSH)GetStockObject(BLACK_BRUSH));
            const wchar_t* buttonText = L"-";
            DrawText(hdc, buttonText, -1, &lpDraw->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            hHeader = ListView_GetHeader(hListView);
            ShowWindow(hHeader, SW_HIDE);
            return TRUE;
        }

        case IDC_AutoBtn: {

            HDC hdc = lpDraw->hDC;

            // Set background color and text color
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, Ti);
            FillRect(hdc, &lpDraw->rcItem, hDarkGreyBrush);
            FrameRect(hdc, &lpDraw->rcItem, (HBRUSH)GetStockObject(BLACK_BRUSH));
            const wchar_t* buttonText = L"Auto";
            DrawText(hdc, buttonText, -1, &lpDraw->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            hHeader = ListView_GetHeader(hListView);
            ShowWindow(hHeader, SW_HIDE);
            return TRUE;
        }
        case     IDC_DELETE: {

            HDC hdc = lpDraw->hDC;

            // Set background color and text color
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, Ti);
            FillRect(hdc, &lpDraw->rcItem, hDarkGreyBrush);
            FrameRect(hdc, &lpDraw->rcItem, (HBRUSH)GetStockObject(BLACK_BRUSH));
            const wchar_t* buttonText = L"<-Delete";
            DrawText(hdc, buttonText, -1, &lpDraw->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            hHeader = ListView_GetHeader(hListView);
            ShowWindow(hHeader, SW_HIDE);
            return TRUE;
        }

        case     IDC_ADD: {

            HDC hdc = lpDraw->hDC;

            // Set background color and text color
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, Ti);
            FillRect(hdc, &lpDraw->rcItem, hDarkGreyBrush);
            FrameRect(hdc, &lpDraw->rcItem, (HBRUSH)GetStockObject(BLACK_BRUSH));
            const wchar_t* buttonText = L"Add";
            DrawText(hdc, buttonText, -1, &lpDraw->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            hHeader = ListView_GetHeader(hListView);
            ShowWindow(hHeader, SW_HIDE);
            return TRUE;
        }

        case     IDC_COLOR: {

            HDC hdc = lpDraw->hDC;

            // Set background color and text color
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, Ti);
            FillRect(hdc, &lpDraw->rcItem, hDarkGreyBrush);
            FrameRect(hdc, &lpDraw->rcItem, (HBRUSH)GetStockObject(BLACK_BRUSH));
            const wchar_t* buttonText = L"Pick Color";
            DrawText(hdc, buttonText, -1, &lpDraw->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            hHeader = ListView_GetHeader(hListView);
            ShowWindow(hHeader, SW_HIDE);
            return TRUE;
        }

        case     IDC_SEARCH: {

            HDC hdc = lpDraw->hDC;

            // Set background color and text color
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, Ti);
            FillRect(hdc, &lpDraw->rcItem, hDarkGreyBrush);
            FrameRect(hdc, &lpDraw->rcItem, (HBRUSH)GetStockObject(BLACK_BRUSH));
            const wchar_t* buttonText = L"Search";
            DrawText(hdc, buttonText, -1, &lpDraw->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            hHeader=ListView_GetHeader(hListView);
             ShowWindow(hHeader, SW_HIDE);
            break;

            return TRUE;
        }

     

        case IDC_COPY_NAME:
        case IDC_COPY_WEBSITE:
        case IDC_COPY_EMAIL:
        case IDC_COPY_USER:
        case IDC_COPY_PASSWORD:
        case IDC_COPY_NOTE:
        case IDC_COPY_COLOR:
        {
            HDC hdc = lpDraw->hDC;

            // Set background color and text color
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, Ti);
            FillRect(hdc, &lpDraw->rcItem, hDarkGreyBrush);
            FrameRect(hdc, &lpDraw->rcItem, (HBRUSH)GetStockObject(BLACK_BRUSH));
            const wchar_t* buttonText = L"Copy";
            DrawText(hdc, buttonText, -1, &lpDraw->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            return TRUE;
        }
        }
    }
    break;


    case WM_CTLCOLORSTATIC://text
    {
        HDC hdcStatic = (HDC)wParam;
        SetBkMode(hdcStatic, TRANSPARENT);
        return (INT_PTR)CreateSolidBrush(RGB(53, 50, 50));
    }

    case WM_CTLCOLORDLG:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORSCROLLBAR:
    case WM_CTLCOLOREDIT:
     
    {
        HDC hdc = (HDC)wParam;
     
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, currentColor); // White text for contrast
        ShowScrollBar(hListView, SB_VERT, FALSE);   // Hide vertical scrollbar
        ShowScrollBar(hListView, SB_HORZ, FALSE);   // Hide horizontal scrollbar
        hHeader = ListView_GetHeader(hListView);
        ShowWindow(hHeader, SW_HIDE);
        return (LRESULT)hDarkGreyBrush;
    }

    case WM_CTLCOLORBTN:
    {
        HDC hdc = (HDC)wParam;
  
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(53, 50, 50)); // White text for contrast
        return (LRESULT)hDarkGreyBrush;
    }

 

    case WM_ERASEBKGND: {
        HDC hdc = (HDC)wParam;
        HBRUSH hBrush = CreateSolidBrush(dark); // Gray-black background
        RECT rect;
        GetClientRect(hWnd, &rect);
        FillRect(hdc, &rect, hBrush);
        DeleteObject(hBrush);
        ShowWindow(hHeader, SW_HIDE);
        SendMessage(hNote, WM_SETREDRAW, TRUE, 0);   // Re-enable redraw
        InvalidateRect(hNote, NULL, TRUE);
        return 1; // Indicate that background has been erased
    }
 
    case WM_SYSCOLORCHANGE: {
     

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
                    if (idx >= 0 && idx < (int)entries.size())
                    {
                        SetWindowTextW(hName, entries[idx].name.c_str());
                        SetWindowTextW(hWebsite, entries[idx].website.c_str());
                        SetWindowTextW(hEmail, entries[idx].email.c_str());
                        SetWindowTextW(hUser, entries[idx].user.c_str());
                        SetWindowTextW(hPassword, entries[idx].password.c_str());
                        SetWindowTextW(hNote, entries[idx].note.c_str());
                        currentColor = entries[idx].color;
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
        ShowScrollBar(hListView, SB_VERT, FALSE);   // Hide vertical scrollbar
        ShowScrollBar(hListView, SB_HORZ, FALSE);   // Hide horizontal scrollbar
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
        //SetFocus(hWnd);
        return 0;
    }

    case WM_LBUTTONUP:
    {
        dragging = false;
        ReleaseCapture();
        return 0;
    }


    case WM_TIMER: {

        if (wParam == ONE) {
            
     
            //50
//            TextOut(hdc, 670, 210, L"X\0", 1);
            animationIndex = (animationIndex + 1) % Tlength;
            RECT rect = { 620, 160, 720, 560 };
             InvalidateRect(hWnd, &rect, TRUE);
           // SetFocus(hTooltip);
        }

    }
    

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);


        // Set color and pen
        HPEN hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0)); // Red border
        HGDIOBJ hOldPen = SelectObject(hdc, hPen);

        RECT clientRect;
        GetClientRect(hWnd, &clientRect);
        int width = clientRect.right - clientRect.left;
        int height = clientRect.bottom - clientRect.top;

        // Create double-buffering DC
        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hbmMem = CreateCompatibleBitmap(hdc, width, height);
        HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

        // Fill background
        HBRUSH hBrush = CreateSolidBrush(dark);
        FillRect(hdcMem, &clientRect, hBrush);
        DeleteObject(hBrush);

        // Draw the bitmap
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
            // Optional: Indicate bitmap not loaded
            TextOutW(hdcMem, 10, 10, L"Bitmap not loaded", 16);
        }
        

        // Copy to window DC
        BitBlt(hdc, 0, 0, width, height, hdcMem, 0, 0, SRCCOPY);

        /*
        HICON ihIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PWRD));
        if (ihIcon) {
            int x = 500;
            int y = 500;
            int cx = GetSystemMetrics(SM_CXICON);
            int cy = GetSystemMetrics(SM_CYICON);
            DrawIconEx(hdc, x, y, ihIcon, cx, cy, 0, nullptr, DI_NORMAL);
        }
        */

        // Select the big font
        HFONT hOldFont = (HFONT)SelectObject(hdc, hBigFont);

        // Set text color and draw
        wchar_t currentChar[2] = { animationChars[animationIndex], L'\0' }; // Use wide-char notation

        SetTextColor(hdc, RGB(11, 11, 11));         //+++++++++++++++++++++
        SetBkMode(hdc, TRANSPARENT);
        TextOut(hdc, 670, 210, currentChar, 1);
         
       // TextOut(hdc, 670, 210, L"X\0", 1);


/////////////////////////////////////
// Create a red pen for the border
        HPEN hPen2 = CreatePen(PS_SOLID, 1, RGB(22,  22, 22));//++++++++++++++++++++++
        HGDIOBJ hOldPen2 = SelectObject(hdc, hPen2);

        // Use a hollow brush (no fill)
        HBRUSH hBrush2 = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
        HGDIOBJ hOldBrush2 = SelectObject(hdc, hBrush2);

        // Draw the rectangle
        Rectangle(hdc, 0, 0, 800, 600);
        hPen2 = CreatePen(PS_SOLID, 1, RGB(11, 11, 11));
        Rectangle(hdc, 1, 1, 799, 599);

        // Restore previous objects
        SelectObject(hdc, hOldPen2);
        SelectObject(hdc, hOldBrush2) ;
        DeleteObject(hPen2);
        /////////////////////////////////


        // Restore the old font
        SelectObject(hdc, hOldFont);

        // Cleanup
        SelectObject(hdcMem, hbmOld);
        DeleteObject(hbmMem);
        DeleteDC(hdcMem);

        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case   IDC_XBtn: {
             DestroyWindow(hWnd);
            break;
        }
        case     IDC_MidBtn: {

            ShowTrayIcon();
            // Hide the window
            ShowWindow(hWnd, SW_HIDE);
           
            break;
        }
        case     IDC_LowBtn: {
            SendMessage(hWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
            break;
        }
        


        case IDC_AutoBtn:
        {
            
            GenerateAndSetPassword(hPassword, 16);
            break;
        }


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
            entry.color = currentColor;

            if (!entry.name.empty()) {
                bool updated = false;
                for (auto& e : entries) {
                    if (_wcsicmp(e.name.c_str(), entry.name.c_str()) == 0) {
                        // Only update if something changed
                        if (e.website != entry.website ||
                            e.email != entry.email ||
                            e.user != entry.user ||
                            e.password != entry.password ||
                            e.note != entry.note ||
                            e.color != entry.color)
                        {
                            e.website = entry.website;
                            e.email = entry.email;
                            e.user = entry.user;
                            e.password = entry.password;
                            e.note = entry.note;
                            e.color = entry.color;
                            SaveXML();
                            PopulateListView();
                        }
                        updated = true;
                        break;
                    }
                }

                if (!updated) {
                    entries.push_back(entry);
                    SaveXML();
                    PopulateListView();
                }

                // Clear fields
                SetWindowTextW(hName, L"");
                SetWindowTextW(hWebsite, L"");
                SetWindowTextW(hEmail, L"");
                SetWindowTextW(hUser, L"");
                SetWindowTextW(hPassword, L"");
                SetWindowTextW(hNote, L"");
                currentColor = RGB(53, 50, 50);
                SetFocus(hWnd);
            }
            break;
        }


        case IDC_DELETE:
        {
            int idx = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
            if (idx >= 0 && idx < (int)entries.size()) {
                entries.erase(entries.begin() + idx);
                SaveXML();
                PopulateListView();
                SetWindowTextW(hName, L"");
                SetWindowTextW(hWebsite, L"");
                SetWindowTextW(hEmail, L"");
                SetWindowTextW(hUser, L"");
                SetWindowTextW(hPassword, L"");
                SetWindowTextW(hNote, L"");
                currentColor = RGB(53, 50, 50);
                SetFocus(hWnd);
            }
            break;
        }

         
        case IDC_SEARCH:
     {
    WCHAR searchText[256];
    GetWindowTextW(hSearchEdit, searchText, 256);
    std::wstring search = searchText;

    ListView_DeleteAllItems(hListView);

    for (size_t i = 0; i < entries.size(); ++i) {
        const PasswordEntry& e = entries[i];
        LVITEMW lvi = { 0 };
        lvi.mask = LVIF_TEXT | LVIF_PARAM;
        lvi.iItem = (int)i;

        // Check match
        if (search.empty() ||
            e.name.find(search) != std::wstring::npos ||
            e.website.find(search) != std::wstring::npos ||
            e.note.find(search) != std::wstring::npos)
        {
            lvi.pszText = const_cast<LPWSTR>(e.name.c_str());  // actual data
            lvi.lParam = (LPARAM)i;
        }
        else {
            lvi.pszText =(LPWSTR) L"------";  // placeholder for non-match
            lvi.lParam = (LPARAM)-1;  // invalid index to skip selection
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

         for (size_t i = 0; i < entries.size(); ++i) {
             const PasswordEntry& e = entries[i];
             LVITEMW lvi = { 0 };
             lvi.mask = LVIF_TEXT | LVIF_PARAM;
             lvi.iItem = (int)i;
             lvi.pszText = const_cast<LPWSTR>(e.name.c_str());  // actual data
             lvi.lParam = (LPARAM)i;
             ListView_InsertItem(hListView, &lvi);
         }

         UpdateListViewColors();

         hHeader = ListView_GetHeader(hListView);
         ShowWindow(hHeader, SW_HIDE);
         ShowScrollBar(hListView, SB_VERT, FALSE);   // Hide vertical scrollbar
         ShowScrollBar(hListView, SB_HORZ, FALSE);   // Hide horizontal scrollbar

         SetFocus(hWnd);
         break;
        }
 

        case IDC_COLOR:
        {
            cc.rgbResult = currentColor;
            cc.Flags = CC_RGBINIT | CC_SOLIDCOLOR; // Basic color dialog only

            if (ChooseColor(&cc)) {
                currentColor = cc.rgbResult;
                int idx = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
                if (idx >= 0 && idx < (int)entries.size()) {
                    entries[idx].color = currentColor;
                    SaveXML();
                    UpdateListViewColors();
                }
                InvalidateRect(hColorBtn, nullptr, TRUE);
                
                    ShowWindow(hHeader, SW_HIDE);
                break;
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
        case IDC_COPY_COLOR:
        {
            WCHAR buffer[32];
            swprintf_s(buffer, L"#%02X%02X%02X", GetRValue(currentColor), GetGValue(currentColor), GetBValue(currentColor));
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
           // Force focus to main window and destroy it
            SetFocus(hWnd);
           DestroyWindow(hWnd);
       }

       ShowScrollBar(hListView, SB_VERT, FALSE);   // Hide vertical scrollbar
       ShowScrollBar(hListView, SB_HORZ, FALSE);   // Hide horizontal scrollbar
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
            entry.color = currentColor;

            bool updated = false;
            bool exists = false;

            for (auto& e : entries) {
                if (e.name == entry.name) {
                    exists = true;

                    // Only update if any field is different
                    if (e.website != entry.website ||
                        e.email != entry.email ||
                        e.user != entry.user ||
                        e.password != entry.password ||
                        e.note != entry.note ||
                        e.color != entry.color)
                    {
                        e = entry;
                        updated = true;
                    }
                    break;
                }
            }

            // If not found, add new
            if (!exists) {
                entries.push_back(entry);
                updated = true;
            }

            if (updated) {
                SaveXML(); // Save only if something changed
            }
        }

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



///////////////////////////////////////////////////////////


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







 

// Convert UTF-8 string to wstring
std::wstring Utf8ToWstring(const char* utf8) {
    if (!utf8) return L"";
    int wideLen = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, nullptr, 0);
    if (wideLen == 0) return L"";
    std::vector<wchar_t> buffer(wideLen);
    MultiByteToWideChar(CP_UTF8, 0, utf8, -1, buffer.data(), wideLen);
    return std::wstring(buffer.data());
}

// Convert wstring to UTF-8 string
std::string WstringToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    int utf8Len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (utf8Len == 0) return "";
    std::vector<char> buffer(utf8Len);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, buffer.data(), utf8Len, nullptr, nullptr);
    return std::string(buffer.data());
}

void LoadXML() {
    entries.clear();
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile("passwords.xml") != tinyxml2::XML_SUCCESS) return;

    tinyxml2::XMLElement* root = doc.FirstChildElement("Passwords");
    if (!root) return;

    for (tinyxml2::XMLElement* entry = root->FirstChildElement("Entry"); entry; entry = entry->NextSiblingElement("Entry")) {
        PasswordEntry pe;
        tinyxml2::XMLElement* nameElem = entry->FirstChildElement("Name");
        pe.name = nameElem && nameElem->GetText() ? Utf8ToWstring(nameElem->GetText()) : L"";
        tinyxml2::XMLElement* websiteElem = entry->FirstChildElement("Website");
        pe.website = websiteElem && websiteElem->GetText() ? Utf8ToWstring(websiteElem->GetText()) : L"";
        tinyxml2::XMLElement* emailElem = entry->FirstChildElement("Email");
        pe.email = emailElem && emailElem->GetText() ? Utf8ToWstring(emailElem->GetText()) : L"";
        tinyxml2::XMLElement* userElem = entry->FirstChildElement("User");
        pe.user = userElem && userElem->GetText() ? Utf8ToWstring(userElem->GetText()) : L"";
        tinyxml2::XMLElement* passwordElem = entry->FirstChildElement("Password");
        pe.password = passwordElem && passwordElem->GetText() ? Utf8ToWstring(passwordElem->GetText()) : L"";
        tinyxml2::XMLElement* noteElem = entry->FirstChildElement("Note");
        pe.note = noteElem && noteElem->GetText() ? Utf8ToWstring(noteElem->GetText()) : L"";
        tinyxml2::XMLElement* colorElem = entry->FirstChildElement("Color");
        unsigned int r = 0, g = 0, b = 0;
        if (colorElem) {
            colorElem->QueryUnsignedAttribute("R", &r);
            colorElem->QueryUnsignedAttribute("G", &g);
            colorElem->QueryUnsignedAttribute("B", &b);
        }
        pe.color = RGB(r, g, b);
        entries.push_back(pe);
    }
}

void SaveXML() {
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLDeclaration* decl = doc.NewDeclaration();
    doc.InsertFirstChild(decl);
    tinyxml2::XMLElement* root = doc.NewElement("Passwords");
    doc.InsertEndChild(root);

    for (const auto& entry : entries) {
        tinyxml2::XMLElement* xmlEntry = doc.NewElement("Entry");
        tinyxml2::XMLElement* name = doc.NewElement("Name");
        name->SetText(WstringToUtf8(entry.name).c_str());
        xmlEntry->InsertEndChild(name);
        tinyxml2::XMLElement* website = doc.NewElement("Website");
        website->SetText(WstringToUtf8(entry.website).c_str());
        xmlEntry->InsertEndChild(website);
        tinyxml2::XMLElement* email = doc.NewElement("Email");
        email->SetText(WstringToUtf8(entry.email).c_str());
        xmlEntry->InsertEndChild(email);
        tinyxml2::XMLElement* user = doc.NewElement("User");
        user->SetText(WstringToUtf8(entry.user).c_str());
        xmlEntry->InsertEndChild(user);
        tinyxml2::XMLElement* password = doc.NewElement("Password");
        password->SetText(WstringToUtf8(entry.password).c_str());
        xmlEntry->InsertEndChild(password);
        tinyxml2::XMLElement* note = doc.NewElement("Note");
        note->SetText(WstringToUtf8(entry.note).c_str());
        xmlEntry->InsertEndChild(note);
        tinyxml2::XMLElement* color = doc.NewElement("Color");
        color->SetAttribute("R", GetRValue(entry.color));
        color->SetAttribute("G", GetGValue(entry.color));
        color->SetAttribute("B", GetBValue(entry.color));
        xmlEntry->InsertEndChild(color);
        root->InsertEndChild(xmlEntry);
    }

    doc.SaveFile("passwords.xml");
}

void PopulateListView() {
    ListView_DeleteAllItems(hListView);
    for (size_t i = 0; i < entries.size(); ++i) {
        LVITEMW lvi = { 0 };
        lvi.mask = LVIF_TEXT | LVIF_PARAM;
        lvi.iItem = (int)i;
        lvi.pszText = const_cast<LPWSTR>(entries[i].name.c_str());
        lvi.lParam = (LPARAM)i;
        ListView_InsertItem(hListView, &lvi);
    }
    UpdateListViewColors();
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
/*
void UpdateListViewColors() {
    for (int i = 0; i < ListView_GetItemCount(hListView); ++i) {
        LVITEMW lvi = { 0 };
        lvi.iItem = i;
        lvi.mask = LVIF_PARAM;
        ListView_GetItem(hListView, &lvi);
        size_t idx = (size_t)lvi.lParam;
        if (idx < entries.size()) {
            ListView_SetTextColor(hListView, i, entries[idx].color);
        }
    }
    InvalidateRect(hListView, nullptr, TRUE);
}*/

/*

// Get the path to pin.x in the executable's directory
BOOL GetPinFilePath(TCHAR* path, size_t pathSize)
{
    if (GetModuleFileName(NULL, path, pathSize) == 0)
        return FALSE;
    PathRemoveFileSpec(path);
    PathAppend(path, TEXT("pin.x"));
    return TRUE;
}

// Read PIN from pin.x and validate it's 6 digits
BOOL ReadPinFromFile(TCHAR* pin, size_t pinSize, BOOL* isValid)
{
    TCHAR filePath[MAX_PATH];
    if (!GetPinFilePath(filePath, MAX_PATH))
        return FALSE;

    HANDLE hFile = CreateFile(filePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return FALSE;

    DWORD bytesRead;
    char filePin[7]; // 6 digits + null terminator
    BOOL success = ReadFile(hFile, filePin, sizeof(filePin) - 1, &bytesRead, NULL);
    CloseHandle(hFile);

    if (success && bytesRead == 6)
    {
        filePin[6] = '\0';
#ifdef UNICODE
        MultiByteToWideChar(CP_ACP, 0, filePin, -1, pin, pinSize);
#else
        strncpy_s(pin, pinSize, filePin, _TRUNCATE);
#endif
        // Validate: exactly 6 digits
        * isValid = (lstrlen(pin) == 6 && StrSpn(pin, TEXT("0123456789")) == 6);
        return TRUE;
    }
    *isValid = FALSE;
    return FALSE;
}
*/
/*
// Write PIN to pin.x
BOOL WritePinToFile(const TCHAR* pin)
{
    TCHAR filePath[MAX_PATH];
    if (!GetPinFilePath(filePath, MAX_PATH))
        return FALSE;

    HANDLE hFile = CreateFile(filePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return FALSE;

    DWORD bytesWritten;
    char filePin[7];
#ifdef UNICODE
    WideCharToMultiByte(CP_ACP, 0, pin, -1, filePin, sizeof(filePin), NULL, NULL);
#else
    strncpy_s(filePin, sizeof(filePin), pin, _TRUNCATE);
#endif
    BOOL success = WriteFile(hFile, filePin, 6, &bytesWritten, NULL);
    CloseHandle(hFile);
    return success && bytesWritten == 6;
}
*/
// Dialog procedure for the password dialog
INT_PTR CALLBACK PasswordDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hwndEdit;
    switch (uMsg)
    {
    case WM_INITDIALOG:
    {
        hwndEdit = GetDlgItem(hwndDlg, IDC_EDIT_PIN);
        if (hwndEdit == NULL)
        {
            MessageBox(hwndDlg, TEXT("Failed to get edit control!"), TEXT("Error"), MB_OK | MB_ICONERROR);
            EndDialog(hwndDlg, IDCANCEL);
            return FALSE;
        }
        SendMessage(hwndEdit, EM_SETLIMITTEXT, 6, 0); // Limit to 6 digits
        SetFocus(hwndEdit);
        SetWindowText(hwndDlg, lParam ? TEXT("Verify PIN") : TEXT("Set New PIN"));
        return FALSE;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
        {
            TCHAR pin[256];
            GetDlgItemText(hwndDlg, IDC_EDIT_PIN, pin, sizeof(pin) / sizeof(pin[0]));

            // Validate: 6 digits
            if (lstrlen(pin) != 6 || !StrSpn(pin, TEXT("0123456789")))
            {
                MessageBox(hwndDlg, TEXT("PIN must be exactly 6 digits!"), TEXT("Error"), MB_OK | MB_ICONERROR);
                SetDlgItemText(hwndDlg, IDC_EDIT_PIN, TEXT(""));
                SetFocus(hwndEdit);
                return TRUE;
            }

            // Store PIN in GWLP_USERDATA
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)_tcsdup(pin));
            EndDialog(hwndDlg, IDOK);
            SecureZeroMemory(pin, sizeof(pin));
            return TRUE;
        }
        case IDCANCEL:
            EndDialog(hwndDlg, IDCANCEL);
            return TRUE;
        }
        break;
    case WM_CLOSE:
        EndDialog(hwndDlg, IDCANCEL);
        return TRUE;
    }
    return FALSE;
}

// Call PIN dialog and handle all pin.x logic
int callpin(HWND hwnd)
{

    /*
    if (IsDebuggerPresent())
    {
        MessageBox(hwnd, TEXT("Debugger detected! PIN verification aborted."), TEXT("Security Error"), MB_OK | MB_ICONERROR);
        return -1;
    }

    TCHAR filePath[MAX_PATH];
    BOOL pinFileExists = FALSE;
    BOOL pinIsValid = FALSE;
    TCHAR storedPin[7] = { 0 };
    TCHAR* enteredPin = NULL;

    // Check if pin.x exists and read PIN
    if (GetPinFilePath(filePath, MAX_PATH))
    {
        pinFileExists = PathFileExists(filePath);
        if (pinFileExists)
        {
            if (!ReadPinFromFile(storedPin, sizeof(storedPin) / sizeof(storedPin[0]), &pinIsValid))
            {
                pinIsValid = FALSE;
            }
        }
    }
     
         
   

    // Show PIN dialog (set new PIN if file doesn't exist or PIN is invalid)
    INT_PTR result = DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_PASSWORD_DIALOG), hwnd, PasswordDlgProc, pinFileExists && pinIsValid);
    if (result == -1)
    {
        TCHAR error[256];
        wsprintf(error, TEXT("DialogBox failed with error %d"), GetLastError());
        MessageBox(hwnd, error, TEXT("Error"), MB_OK | MB_ICONERROR);
        return -1;
    }

    if (result != IDOK)
    {
        MessageBox(hwnd, TEXT("PIN entry cancelled."), TEXT("Error"), MB_OK | MB_ICONERROR);
        return -1;
    }



    

    // Handle PIN logic
    int returnValue = -1;
    if (!pinFileExists || !pinIsValid)
    {
        // Save new PIN to pin.x
        if (!WritePinToFile(enteredPin))
        {
            MessageBox(hwnd, TEXT("Failed to save PIN to pin.x!"), TEXT("Error"), MB_OK | MB_ICONERROR);
        }
        else
        {
            returnValue = 1; // Success: new PIN saved
        }
    }
    else
    {
        // Verify PIN
        if (lstrcmp(enteredPin, storedPin) == 0)
        {
            returnValue = 1; // Success: PIN matches
        }
        else
        {
            MessageBox(hwnd, TEXT("PIN does not match!"), TEXT("Error"), MB_OK | MB_ICONERROR);
        }
    }

    // Cleanup
    free(enteredPin);
    SecureZeroMemory(storedPin, sizeof(storedPin));
    return returnValue;
    */

    return 0;
}
