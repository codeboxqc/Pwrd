#define WIN32_LEAN_AND_MEAN

// Add to your project settings or code
//#ifndef NTDDI_VERSION
#define NTDDI_VERSION NTDDI_WIN10
//#endif

#define _WIN32_WINNT _WIN32_WINNT_WIN10

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
#include <windows.h>
#include <gdiplus.h>
#include <wincrypt.h>
#include <bcrypt.h> 
#include "tinyxml2.h"
#include <tlhelp32.h> // Include this header for TH32CS_SNAPPROCESS and CreateToolhelp32Snapshot

#include <wintrust.h>
#include <softpub.h>
#pragma comment(lib, "wintrust.lib")

#pragma comment(lib, "Kernel32.lib") // Link against Kernel32.lib
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "comctl32.lib") 
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shlwapi.lib")

#include <winternl.h>
#pragma comment(lib, "ntdll.lib")


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
HWND AutoBtn2 = nullptr;
HWND hStrengthLabel = nullptr, hTogglePasswordBtn = nullptr, hSortCombo = nullptr, hCategory = nullptr, hRestoreBackupBtn = nullptr, hToggleThemeBtn = nullptr;
COLORREF currentColor = RGB(222, 222, 8);
COLORREF dark = RGB(33, 33, 33);
COLORREF textColor = RGB(255, 255, 255);
static bool isDarkTheme = true;
static CHOOSECOLOR cc = { 0 };
static COLORREF customColors[16] = { 0 };
HBRUSH hDarkGreyBrush = nullptr;
HBRUSH butBrush = nullptr;

////////////////////
std::vector<PasswordEntry> entries;

//struct WindowData {
//    std::vector<PasswordEntry> entries;
 //   int lastSelectedIndex = -1;
///////////////////////////


static int g_lastSelectedEntryIndex = -1;
static bool g_dataModifiedInFields = false;
static bool g_isInsideApplyChanges = false;
static bool isPasswordVisible = false;
bool resto = 0;
std::wstring files;
HWND hToggleStartupBtn = nullptr; // Handle for the new toggle button
bool g_isStartupEnabled = false; // Tracks current startup state

bool DisableStartup();
bool IsStartupEnabled();
bool EnableStartup();
int mini = 0;
// Global variable
#define AUTO_LOCK_TIMER 2 // Distinct from ONE (1)
static UINT_PTR g_lockTimer = 0;

#define CLIPBOARD_CLEAR_TIMER 3 // Distinct from ONE (1) and AUTO_LOCK_TIMER (2)
static UINT_PTR g_clipboardTimer = 0;

bool IsValidInput(const std::wstring& input);

bool updown = true;

static std::wstring g_enteredPassword; // Added for storing the password from the new dialog
static bool g_isInVerifyModeNewDialog = false; // To store the mode for the new dialog

// Registry key for startup
#define REG_RUN_KEY L"Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#define APP_NAME L"Pwrd"

/*
const wchar_t* animationSets[] = {
    L"/-\\|\0",
    L"⠋⠙⠹⠸⠼⠽⠾⠿\0",
    L"←↖↑↗→↘↓↙\0",
    L"█▓▒░▒▓█\0",
    L"⏳⌛\0"
};
*/

const wchar_t* animationSets[] = {
    L"/-\\|\0",  // Classic spinner
    L"⠋⠙⠹⠸⠼⠽⠾⠿\0",  // Braille dots
    L"←↖↑↗→↘↓↙\0",  // Directional arrows
    L"█▓▒░▒▓█\0",  // Shading effect
    L"⏳⌛\0",  // Hourglass flip
    L".oO0Oo\0",       // Pulsing dots to circle
    L"12321\0",      // Number countdown
    L"oO0Oo\0",      // Circle pulse
    L"/-\\|\0",  // Classic spinner


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


bool IsVMCPU();
bool IsDebuggerAttached();
bool IsRunningInVM();
bool IsBeingDebugged();
bool OutputDebugStringCheck();
bool NtQueryDebugFlag();
bool CheckForBreakpointsInCode();
bool VerifyExecutableIntegrity();

bool mili = 0;

void CopyToClipboard(HWND hWnd, const std::wstring& text);
void GenerateSecurePassword(HWND hEdit,
    size_t length = 16,
    bool skipAmbiguous = true,
    bool militaryGrade = false);


INT_PTR CALLBACK AboutNewDlgProc(HWND, UINT, WPARAM, LPARAM);

#define CLIPBOARD_CLEAR_TIMER_ID 901
#define CLIPBOARD_CLEAR_DELAY_MS 30000 // 30 seconds

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

void UpdateListViewColors();
void UpdatePasswordStrength(HWND hWnd, HWND hPasswordEdit);
int cEXIST(HWND hWnd);
bool SecureDeleteFile(const std::wstring& filePath);

INT_PTR CALLBACK PasswordDialogProcNew(HWND, UINT, WPARAM, LPARAM); // Added



void StartClipboardClearTimer(HWND hWnd) {
    SetTimer(hWnd, CLIPBOARD_CLEAR_TIMER_ID, CLIPBOARD_CLEAR_DELAY_MS, nullptr);
}

void StopClipboardClearTimer(HWND hWnd) {
    KillTimer(hWnd, CLIPBOARD_CLEAR_TIMER_ID);
}




//  function to reset the autolock timer
void ResetAutoLockTimer(HWND hWnd) {
    if (g_lockTimer) KillTimer(hWnd, AUTO_LOCK_TIMER);
    g_lockTimer = SetTimer(hWnd, AUTO_LOCK_TIMER, 5 * 60 * 1000, NULL); // 5 minutes

}



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



void ClearSensitiveDataAndUI(HWND hWnd, bool preserveListView = false) {
    if (!g_userKeyForXml.empty()) {
        SecureZeroMemory(g_userKeyForXml.data(), g_userKeyForXml.size());
        g_userKeyForXml.clear();
        g_userKeyForXml.shrink_to_fit();
    }

    entries.clear();
    g_isPinValidated = false;
    g_pin_attempts = 0;
    if (hName) SetWindowTextW(hName, L"");
    if (hWebsite) SetWindowTextW(hWebsite, L"");
    if (hEmail) SetWindowTextW(hEmail, L"");
    if (hUser) SetWindowTextW(hUser, L"");
    if (hPassword) SetWindowTextW(hPassword, L"");
    if (hNote) SetWindowTextW(hNote, L"");
    if (hCategory) SetWindowTextW(hCategory, L"");
    if (hListView && !preserveListView) ListView_DeleteAllItems(hListView);
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


/*
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
*/


/*
void GenerateSecurePassword(HWND hEdit,
    size_t length = 16,
    bool  skipAmbiguous = true)
{
    // ---------- character pools ----------
    const std::wstring UPPER = L"ABCDEFGHJKLMNPQRSTUVWXYZ";           // no I or O
    const std::wstring LOWER = L"abcdefghijkmnopqrstuvwxyz";          // no l
    const std::wstring DIGIT = L"23456789";                           // no 0 or 1
    const std::wstring SYM = L"!@#$%^&*()-_=+[]{}<>?/|";

    std::wstring allPool = UPPER + LOWER + DIGIT + SYM;
    if (!skipAmbiguous) {
        allPool += L"IOl01|"; // add back the ambiguities if caller wants
    }

    auto rndByte = [](BYTE* buf, DWORD len) {
        BCryptGenRandom(nullptr, buf, len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
        };

    // ---------- guarantee one from each set ----------
    std::wstring pwd;
    BYTE b;
    rndByte(&b, 1);     pwd += UPPER[b % UPPER.size()];
    rndByte(&b, 1);     pwd += LOWER[b % LOWER.size()];
    rndByte(&b, 1);     pwd += DIGIT[b % DIGIT.size()];
    rndByte(&b, 1);     pwd += SYM[b % SYM.size()];

    // ---------- fill the rest ----------
    while (pwd.size() < length) {
        rndByte(&b, 1);
        pwd += allPool[b % allPool.size()];
    }

    // ---------- Fisher‑Yates shuffle ----------
    for (size_t i = pwd.size() - 1; i > 0; --i) {
        rndByte(&b, 1);
        size_t j = b % (i + 1);
        std::swap(pwd[i], pwd[j]);
    }

    SetWindowTextW(hEdit, pwd.c_str());
    UpdatePasswordStrength(hEdit, hEdit);
}
*/

// Optional: Military grade password with predefined settings
void GenerateMilitaryGradePassword(HWND hEdit, size_t length = 32) {
    GenerateSecurePassword(hEdit, length, true, true);
}

// Optional: Get password entropy estimate
double CalculatePasswordEntropy(size_t length, size_t characterSetSize) {
    return length * (std::log2(static_cast<double>(characterSetSize)));
}


void GenerateSecurePassword(HWND hEdit,
    size_t length,
    bool skipAmbiguous,
    bool militaryGrade)
{
    // ---------- Enhanced character pools ----------
    const std::wstring UPPER = L"ABCDEFGHJKLMNPQRSTUVWXYZ";           // no I or O
    const std::wstring LOWER = L"abcdefghijkmnopqrstuvwxyz";          // no l
    const std::wstring DIGIT = L"23456789";                           // no 0 or 1
    const std::wstring SYM = L"!@#$%^&*()-_=+[]{}<>?/|\\~`;:\"'.,";   // Extended symbols

    // Military grade additions
    //Best Practice
    //Use extended symbols only if the target website allows it.
    const std::wstring EXTENDED_SYM = L"§±¿¡€£¥©®™°µ¶";              // Unicode symbols
    const std::wstring MATH_SYM = L"∑∏∆∇∈∉∪∩⊂⊃⊄⊅";               // Mathematical symbols

    std::wstring allPool = UPPER + LOWER + DIGIT + SYM;

    if (militaryGrade) {
        allPool += EXTENDED_SYM + MATH_SYM;
        length = std::max(length, size_t(24)); // Minimum 24 chars for military grade
    }

    if (!skipAmbiguous) {
        allPool += L"IOl01|"; // add back the ambiguities if caller wants
    }

    // ---------- Enhanced random number generation ----------
    auto rndBytes = [](BYTE* buf, DWORD len) {
        NTSTATUS status = BCryptGenRandom(nullptr, buf, len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
        if (!BCRYPT_SUCCESS(status)) {
            throw std::runtime_error("Cryptographic random generation failed");
        }
        };

    // Get entropy from multiple sources for military grade
    auto getSecureRandom = [&](size_t max) -> size_t {
        if (militaryGrade) {
            // Use 4 bytes for better distribution with large character sets
            DWORD entropy;
            rndBytes(reinterpret_cast<BYTE*>(&entropy), sizeof(entropy));
            return entropy % max;
        }
        else {
            BYTE b;
            rndBytes(&b, 1);
            return b % max;
        }
        };

    // ---------- Guarantee character distribution ----------
    std::wstring pwd;

    // Ensure at least one from each required category
    pwd += UPPER[getSecureRandom(UPPER.size())];
    pwd += LOWER[getSecureRandom(LOWER.size())];
    pwd += DIGIT[getSecureRandom(DIGIT.size())];
    pwd += SYM[getSecureRandom(SYM.size())];

    if (militaryGrade) {
        // Additional guaranteed characters for military grade
        pwd += EXTENDED_SYM[getSecureRandom(EXTENDED_SYM.size())];
        pwd += MATH_SYM[getSecureRandom(MATH_SYM.size())];

        // Ensure multiple instances of each category for longer passwords
        if (length >= 32) {
            pwd += UPPER[getSecureRandom(UPPER.size())];
            pwd += DIGIT[getSecureRandom(DIGIT.size())];
        }
    }

    // ---------- Fill remaining characters ----------
    while (pwd.size() < length) {
        pwd += allPool[getSecureRandom(allPool.size())];
    }

    // ---------- Enhanced Fisher-Yates shuffle ----------
    // Multiple passes for military grade
    int shufflePasses = militaryGrade ? 3 : 1;

    for (int pass = 0; pass < shufflePasses; ++pass) {
        for (size_t i = pwd.size() - 1; i > 0; --i) {
            size_t j = getSecureRandom(i + 1);
            std::swap(pwd[i], pwd[j]);
        }
    }

    // ---------- Additional entropy injection for military grade ----------
    if (militaryGrade) {
        // Randomly replace a few characters to break any potential patterns
        size_t replacements = std::min(length / 8, size_t(4));
        for (size_t r = 0; r < replacements; ++r) {
            size_t pos = getSecureRandom(pwd.size());
            pwd[pos] = allPool[getSecureRandom(allPool.size())];
        }
    }

    // ---------- Security cleanup ----------
    SetWindowTextW(hEdit, pwd.c_str());

    // Clear password from memory (military grade practice)
    if (militaryGrade) {
        SecureZeroMemory(const_cast<wchar_t*>(pwd.data()), pwd.size() * sizeof(wchar_t));
    }

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


    ////////////////////////////////////////////
      //Prevent DLL Injection
    SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_SYSTEM32);


    //Check for Debuggers
    if (IsDebuggerPresent()) {
        // MessageBoxW(nullptr, L"Debugger detected. Exiting.", L"Security Alert", MB_OK | MB_ICONERROR);
        return FALSE;
    }



     //   if(IsVMCPU()==1) return FALSE;

     if (IsDebuggerAttached() == true) return FALSE;

        if (IsRunningInVM() == true) return FALSE;

      if (IsBeingDebugged() == true) return FALSE;  

      if (OutputDebugStringCheck() == true) return FALSE;

        if (NtQueryDebugFlag() == true) return FALSE;

       
   

   /////////////////////////////////////////////////////
       // Create a named mutex to check for existing instance
    HANDLE hMutex = CreateMutexW(nullptr, TRUE, L"Pwrd");
    if (hMutex == nullptr || GetLastError() == ERROR_ALREADY_EXISTS) {
        // Another instance is already running
        MessageBoxW(nullptr, L"Application is already running!", L"Error", MB_OK | MB_ICONERROR);
        if (hMutex) {
            CloseHandle(hMutex);
        }
        return FALSE;
    }
    ///////////////////////////////

    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    PathRemoveFileSpecW(exePath);

    //tooltips

    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icex);

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


    mini = cEXIST(hWnd);
    if (mini == 0) {
        KillTrayIcon();
        ShowWindow(hWnd, SW_HIDE);
        PostQuitMessage(0);
        return FALSE;

    }

    //////////////////////////////


    if (!hWnd)
    {
        WCHAR errorMsg[256];
        swprintf_s(errorMsg, L"Failed to create window! Error code: %lu", GetLastError());
        MessageBoxW(nullptr, errorMsg, L"Error", MB_OK | MB_ICONERROR);
        return FALSE;
    }



    //ShowTrayIcon();
    if (mini == 1) {
        ShowWindow(hWnd, nCmdShow);
        ShowWindow(hWnd, SW_SHOW);
    }
    UpdateWindow(hWnd);
    return TRUE;
}




void AddTooltip(HWND hTooltip, HWND hWnd, HWND hControl, LPCWSTR text)
{
    if (!hTooltip || !hControl) return;

    TOOLINFOW ti = { sizeof(TOOLINFOW) };
    ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    ti.hwnd = hWnd;
    ti.uId = (UINT_PTR)hControl;
    ti.lpszText = const_cast<LPWSTR>(text);  // Safe in this case
    ti.hinst = GetModuleHandle(nullptr);

    // Don't need GetClientRect for TTF_IDISHWND mode
    // GetClientRect(hControl, &ti.rect);  // Only needed if not using TTF_IDISHWND

    SendMessageW(hTooltip, TTM_ADDTOOLW, 0, (LPARAM)&ti);
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
    updatedEntry.creationDate = entries[entryIndex].creationDate;

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
    if (password.length() >= 12) score++;

    if (std::any_of(password.begin(), password.begin(), ::iswupper)) score++;
    if (std::any_of(password.begin(), password.end(), ::iswlower)) score++;
    if (std::any_of(password.begin(), password.end(), ::iswdigit)) score++;
    if (std::any_of(password.begin(), password.end(), [](wchar_t c) { return wcschr(L"!@#$%^&*()-_=+[]{}<>?/|", c); })) score++;

    if (password.length() < 6) score -= 3;
    if (password.find(L"123") != std::wstring::npos) score = std::max(0, score - 2);

    const wchar_t* strength;
    COLORREF strengthColor;

    if (score <= 2) {
        strength = L"Weak";
        strengthColor = RGB(255, 0, 0);
    }
    else if (score == 3) {
        strength = L"Poor";
        strengthColor = RGB(255, 140, 0);
    }
    else if (score == 4) {
        strength = L"Okay";
        strengthColor = RGB(0, 128, 0);
    }
    else {
        strength = L"Strong";
        strengthColor = RGB(0, 200, 255);
    }

    // Get creation date for selected entry
    std::wstring creationDate = L"N/A";
    int idx = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
    if (idx >= 0 && static_cast<size_t>(idx) < entries.size()) {
        creationDate = entries[idx].creationDate.empty() ? L"N/A" : entries[idx].creationDate;
    }

    // Combine strength and creation date
    WCHAR displayText[2048];
    swprintf_s(displayText, L"  %s |  %s ", strength, creationDate.c_str());
    SetWindowTextW(hStrengthLabel, displayText);

    HDC hdc = GetDC(hStrengthLabel);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, strengthColor);
    ReleaseDC(hStrengthLabel, hdc);
    InvalidateRect(hStrengthLabel, nullptr, TRUE);
}


void ini(HWND hWnd)
{


    animationChars = getRandomAnimationSet();


    hListView = CreateWindowEx(0, WC_LISTVIEW, L"",
        // WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_NOCOLUMNHEADER, // Added LVS_NOCOLUMNHEADER
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
        216, 232, 30, 20, hWnd, (HMENU)IDC_AutoBtn, hInst, nullptr);
    AutoBtn2 = CreateWindow(L"BUTTON", L"Mili", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        249, 232, 30, 20, hWnd, (HMENU)IDC_AutoBtn2, hInst, nullptr);


    // hTogglePasswordBtn = CreateWindow(L"BUTTON", L"Show", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
    //     570, 212, 70, 28, hWnd, (HMENU)IDC_TOGGLE_PASSWORD, hInst, nullptr);

    // hPassword = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_PASSWORD,
    //    310, 210, 250, 28, hWnd, (HMENU)IDC_PASSWORD, hInst, nullptr);

    hPassword = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        310, 210, 300, 28, hWnd, (HMENU)IDC_PASSWORD, hInst, nullptr);


    hCopyPasswordBtn = CreateWindow(L"BUTTON", L"Copy", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        654, 210, 50, 20, hWnd, (HMENU)IDC_COPY_PASSWORD, hInst, nullptr);


    hStrengthLabel = CreateWindow(L"STATIC", L" : ", WS_CHILD | WS_VISIBLE,
        310, 250, 220, 20, hWnd, (HMENU)IDC_STRENGTH_LABEL, hInst, nullptr);   //date strength

    // Category field (increased spacing: 60 from previous to avoid overlap)
    CreateWindow(L"STATIC", L"Category", WS_CHILD | WS_VISIBLE,
        220, 300, 80, 20, hWnd, nullptr, hInst, nullptr);
    hCategory = CreateWindow(L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWN | CBS_AUTOHSCROLL,
        300, 300, 350, 200, hWnd, (HMENU)IDC_CATEGORY, hInst, nullptr);
    SendMessage(hCategory, CB_ADDSTRING, 0, (LPARAM)L"Personal");
    SendMessage(hCategory, CB_ADDSTRING, 0, (LPARAM)L"Work");
    SendMessage(hCategory, CB_ADDSTRING, 0, (LPARAM)L"Web");
    SendMessage(hCategory, CB_ADDSTRING, 0, (LPARAM)L"Email");
    SendMessage(hCategory, CB_ADDSTRING, 0, (LPARAM)L"Note");
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
        220, 565, 70, 20, hWnd, (HMENU)IDC_RESTORE_BACKUP, hInst, nullptr);
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
        hWnd, (HMENU)333, hInst, nullptr
    );
    SendMessage(hBtnicon, BM_SETIMAGE, IMAGE_ICON, (LPARAM)tmphIcon);

    // Tooltip setup

    hTooltip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, nullptr,
        WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON | TTS_NOPREFIX,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        hWnd, nullptr, hInst, nullptr);


    if (hTooltip)
    {
        // Set tooltip to be topmost
        SetWindowPos(hTooltip, HWND_TOPMOST, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

        SendMessageW(hTooltip, TTM_SETMAXTIPWIDTH, 0, 300);
        SendMessageW(hTooltip, TTM_SETDELAYTIME, TTDT_AUTOPOP, MAKELPARAM(500, 0));
        SendMessageW(hTooltip, TTM_SETDELAYTIME, TTDT_INITIAL, MAKELPARAM(5000 * 20, 0));
        SendMessageW(hTooltip, TTM_SETDELAYTIME, TTDT_RESHOW, MAKELPARAM(100, 0));



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
        AddTooltip(hTooltip, hWnd, AutoBtn2, L"Military Grade random password");
        AddTooltip(hTooltip, hWnd, hTogglePasswordBtn, L"Toggle password visibility");
        AddTooltip(hTooltip, hWnd, hSortCombo, L"Sort entries by name or category");
        AddTooltip(hTooltip, hWnd, hRestoreBackupBtn, L"Restore from backup file");
        AddTooltip(hTooltip, hWnd, hToggleThemeBtn, L"Switch between dark and light themes");
        AddTooltip(hTooltip, hWnd, hToggleStartupBtn, L"Toggle whether the application starts with Windows");



        SendMessageW(hTooltip, TTM_ACTIVATE, TRUE, 0);
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
    if (hFont) DeleteObject(hFont);
    if (hFont2) DeleteObject(hFont2);



}



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HINSTANCE hInstance_WndProc = NULL;
    static bool isMinimizedState = false;
    static HWND hHeader = nullptr;
    size_t keySize = 32;
    // BYTE* pKey = new BYTE[keySize];
    BYTE* pKey = nullptr;

    // Allocate large variables on the heap instead of the stack
   // std::unique_ptr<WCHAR[]> buffer(new WCHAR[2048]); // Example of moving a large array to

    switch (message)
    {

    case WM_INPUTLANGCHANGE:
        // Prevent screenshots
        SetWindowDisplayAffinity(hWnd, WDA_EXCLUDEFROMCAPTURE);
        return TRUE;


    case WM_CREATE:
    {
        InitCommonControls();
        HRESULT hr = CoInitialize(nullptr);
        if (FAILED(hr)) {
            MessageBoxW(nullptr, L"Failed to initialize COM library.", L"Error", MB_OK | MB_ICONERROR);
            // return FALSE; // Exit or handle the error as needed
        }

        hr = OleInitialize(nullptr);
        if (FAILED(hr)) {
            MessageBoxW(nullptr, L"Failed to initialize OLE. The application will now exit.", L"Error", MB_OK | MB_ICONERROR);
            // return FALSE; // Exit the application if OLE initialization fails
        }

        InitializeGDIPlus();


        if (!VerifyExecutableIntegrity()) {
           // MessageBoxW(hWnd, L"Executable integrity verification failed.", L"Security Error", MB_OK | MB_ICONERROR);
            //  return 0;
        }



        BYTE* pKey = new BYTE[keySize];
        SecureZeroMemory(pKey, 32);
        VirtualLock(pKey, keySize); // Prevent swapping to disk

        


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



        // Prevent screenshots from the start
        SetWindowDisplayAffinity(hWnd, WDA_EXCLUDEFROMCAPTURE);


        ini(hWnd);
        ShowScrollBar(hListView, SB_VERT, FALSE);
        ShowScrollBar(hListView, SB_HORZ, FALSE);
        hHeader = ListView_GetHeader(hListView);
        CreateTrayIcon(hWnd, hInstance_WndProc, IDI_PWRD);

        // Start autolock timer on creation
        ResetAutoLockTimer(hWnd);

        // ClearSensitiveDataAndUI(hWnd);
        // ShowWindow(hWnd, SW_HIDE);
        if (mini == 2) {
            ShowTrayIcon(); // Ensure tray icon is visible
            ShowWindow(hWnd, SW_HIDE);
            updown = false; // Align with existing tray icon logic
        }


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
                    
                    SaveXML(); // Save to persist the color change
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
        if (hTooltip) ShowWindow(hTooltip, SW_SHOW);

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
        case IDC_AutoBtn2:
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
            case IDC_AutoBtn2: buttonText = L"MILI"; break;
            case IDC_DELETE: buttonText = L"<-Delete"; break;
            case IDC_ADD: buttonText = L"Add"; break;
            case IDC_COLOR: buttonText = L"Pick Color"; break;
            case IDC_SEARCH: buttonText = L"Search"; break;
                //case IDC_TOGGLE_PASSWORD: buttonText = isPasswordVisible ? L"Hide" : L"Show"; break;
            case IDC_RESTORE_BACKUP: buttonText = L"Restore"; break;
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


        if (pnm->code == TTN_GETDISPINFO && hTooltip) {
            SendMessageW(hTooltip, WM_NOTIFY, wParam, lParam);
        }


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

        ResetAutoLockTimer(hWnd); // Reset timer on mouse move
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

        if (wParam == AUTO_LOCK_TIMER) {
            ClearSensitiveDataAndUI(hWnd);
            ShowWindow(hWnd, SW_HIDE);
            ShowTrayIcon(); // Ensure tray icon is visible
            ShowWindow(hWnd, SW_HIDE);
            updown = false; // Align with existing tray icon logic
            KillTimer(hWnd, AUTO_LOCK_TIMER);
            updown = false;
            g_lockTimer = 0;
        }

        if (wParam == CLIPBOARD_CLEAR_TIMER_ID) {
            OpenClipboard(hWnd);
            EmptyClipboard();
            CloseClipboard();
            KillTimer(hWnd, CLIPBOARD_CLEAR_TIMER_ID);
        }


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

        case 333:
            //MessageBoxW(hWnd, L"hell-o", L"Debug", MB_OK | MB_ICONINFORMATION);
            //DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUT_NEW), hWnd, AboutNewDlgProc);
            ShellExecuteW(hWnd, L"open", L"https://www.nutz.club", NULL, NULL, SW_SHOWNORMAL);


            break;

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
            //KillTrayIcon();
            //DestroyWindow(hWnd);
            ShowTrayIcon(); // Ensure tray icon is visible
            ShowWindow(hWnd, SW_HIDE);
            updown = false; // Align with existing tray icon logic
            break;
        case IDC_MidBtn:


            ShowTrayIcon();
            updown = false;
            ShowWindow(hWnd, SW_HIDE);

            break;
        case IDC_LowBtn:
            SendMessage(hWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
            break;

         

            
        case IDC_AutoBtn2:
            if (mili == 0) mili = 1;
            else mili = 0;
            break;

        case IDC_AutoBtn:
            //GnerateAndSetPassword(hPassword, 16);

            if (mili == 1) GenerateSecurePassword(hPassword, 32, true, true);
            else GenerateSecurePassword(hPassword, 16, true, false);

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

            // Set creation date for new entry
            SYSTEMTIME st;
            GetLocalTime(&st);
            WCHAR dateBuffer[20];
            swprintf_s(dateBuffer, L"%02d-%02d-%04d %02d:%02d:%02d",
                st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond);
            entry.creationDate = dateBuffer;

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








        /////////////////////////
        case IDC_RESTORE_BACKUP:
        {
            // Confirm restore action
            if (MessageBoxW(hWnd, L"Load save file temps!", L"Confirm Restore", MB_YESNO | MB_ICONQUESTION) != IDYES)  break;


            resto = 1;

            // Initialize OPENFILENAME structure
            WCHAR szFile[MAX_PATH] = { 0 };
            OPENFILENAMEW ofn = { 0 };
            ofn.lStructSize = sizeof(OPENFILENAMEW);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFilter = L"XML Files (*.xml)\0*.xml\0All Files (*.*)\0*.*\0";
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = MAX_PATH;
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
            ofn.lpstrDefExt = L"xml";

            // Show file open dialog
            GetOpenFileNameW(&ofn);

            ListView_DeleteAllItems(hListView);
            cEXIST(hWnd);
            PopulateListView();

            break;
        }
        ////////////////////////////////////////////////////////////     

        case IDC_TOGGLE_THEME:
        {
            isDarkTheme = !isDarkTheme;
            dark = isDarkTheme ? RGB(33, 33, 33) : RGB(22, 22, 22);
            textColor = isDarkTheme ? RGB(255, 255, 255) : RGB(222, 222, 8);
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
                    entries[idx].color = currentColor; // Update the color for the selected entry

                    UpdateListViewColors(); // Refresh ListView to show new color
                    InvalidateRect(hListView, nullptr, TRUE); // Force ListView redraw
                }
                InvalidateRect(hColorBtn, nullptr, TRUE); // Redraw color button
                HWND hHeader = ListView_GetHeader(hListView);
                if (hHeader) ShowWindow(hHeader, SW_HIDE); // Hide header as per existing logic
            }
            break;
        }

        case CDDS_ITEMPREPAINT:
        {
            LVITEMW lvi = { 0 };
            LPNMLVCUSTOMDRAW lplvcd = reinterpret_cast<LPNMLVCUSTOMDRAW>(lParam);
            lvi.iItem = static_cast<int>(lplvcd->nmcd.dwItemSpec);
            lvi.mask = LVIF_PARAM;
            ListView_GetItem(hListView, &lvi);
            size_t idx = (size_t)lvi.lParam;
            if (idx < entries.size())
            {
                lplvcd->clrText = entries[idx].color; // Uses individual entry color
            }
            lplvcd->clrTextBk = dark;
            return CDRF_DODEFAULT;
        }

        case IDC_COPY_NAME:
        {
            WCHAR buffer[1024];
            GetWindowTextW(hName, buffer, 1024);
            CopyToClipboard(hWnd, buffer);
            break;
        }
        case IDC_COPY_WEBSITE:
        {
            WCHAR buffer[1024];
            GetWindowTextW(hWebsite, buffer, 1024);
            CopyToClipboard(hWnd, buffer);
            break;
        }
        case IDC_COPY_EMAIL:
        {
            WCHAR buffer[1024];
            GetWindowTextW(hEmail, buffer, 1024);
            CopyToClipboard(hWnd, buffer);
            break;
        }
        case IDC_COPY_USER:
        {
            WCHAR buffer[1024];
            GetWindowTextW(hUser, buffer, 1024);
            CopyToClipboard(hWnd, buffer);
            break;
        }
        case IDC_COPY_PASSWORD:
        {
            WCHAR buffer[1024];
            GetWindowTextW(hPassword, buffer, 1024);
            CopyToClipboard(hWnd, buffer);
            break;
        }
        case IDC_COPY_NOTE:
        {
            WCHAR buffer[1024];
            GetWindowTextW(hNote, buffer, 1024);
            CopyToClipboard(hWnd, buffer);
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

        ResetAutoLockTimer(hWnd);

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

        SecureZeroMemory(pKey, keySize);
        VirtualUnlock(pKey, keySize);
        delete[] pKey;

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

        SaveXML();
        ShutdownGDIPlus();
        CoUninitialize();
        if (g_clipboardTimer) KillTimer(hWnd, g_clipboardTimer);
        if (ONE) KillTimer(hWnd, ONE);
        if (AUTO_LOCK_TIMER) KillTimer(hWnd, AUTO_LOCK_TIMER);
        if (hBigFont)  DeleteObject(hBigFont);
        
        if (hBitmap)      DeleteObject(hBitmap);
        if (hCustomCursor)        DeleteObject(hCustomCursor);
        if (hDarkGreyBrush)        DeleteObject(hDarkGreyBrush);
        if (butBrush)      DeleteObject(butBrush);
        if (hToggleStartupBtn) DestroyWindow(hToggleStartupBtn);

  
        if (hTooltip)
        {
            DestroyWindow(hTooltip);
            hTooltip = nullptr;
        }

        KillTrayIcon();
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

// Converts a std::wstring to a UTF-8 encoded std::string
std::string StringToUTF8(const std::wstring& wideString) {
    if (wideString.empty()) return std::string();

    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wideString.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (size_needed == 0) return std::string();

    std::string result(size_needed - 1, 0); // Exclude null terminator
    WideCharToMultiByte(CP_UTF8, 0, wideString.c_str(), -1, result.data(), size_needed, nullptr, nullptr);

    return result;
}


tinyxml2::XMLDocument tiny;



 



 


bool LoadXML(std::wstring xmlPath)
{
    WCHAR debugMsg[512];

    // Check if resto == 1 and xmlPath is data.xml, then reset resto to 0
    if (resto == 1 && xmlPath == GetFullFilePath(L"data.xml")) {
        resto = 0;
    }

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

    // Decrypt g_userKeyForXml
    std::vector<BYTE> tempKey = g_userKeyForXml; // Copy to avoid modifying original
    if (!tempKey.empty()) {
        DWORD cbData = static_cast<DWORD>(tempKey.size());
        if (!CryptUnprotectMemory(tempKey.data(), cbData, CRYPTPROTECTMEMORY_SAME_PROCESS)) {
            swprintf_s(debugMsg, L"Failed to decrypt key in memory. Error code: %lu", GetLastError());
            MessageBoxW(nullptr, debugMsg, L"Error", MB_OK | MB_ICONERROR);
            SecureZeroMemory(tempKey.data(), tempKey.size());
            tempKey.clear();
            return false;
        }

        // Decrypt to temp file
        std::wstring tempPath = GetFullFilePath(L"temp_decrypted.xml");
        if (!DecryptFile(xmlPath, tempPath, std::wstring(tempKey.begin(), tempKey.end()))) {
            //DeleteFileW(tempPath.c_str());
            SecureDeleteFile(tempPath.c_str());
            SecureZeroMemory(tempKey.data(), tempKey.size());
            tempKey.clear();
            return false;
        }

        // Load decrypted XML
        tinyxml2::XMLDocument doc;
        doc.Clear();
        std::string tempPathUtf8 = WstringToUtf8(tempPath);
        if (doc.LoadFile(tempPathUtf8.c_str()) != tinyxml2::XML_SUCCESS) {
            //DeleteFileW(tempPath.c_str());
            SecureDeleteFile(tempPath.c_str());
            SecureZeroMemory(tempKey.data(), tempKey.size());
            tempKey.clear();
            return false;
        }

        entries.clear();

        // Parse XML and populate entries
        tinyxml2::XMLElement* root = doc.FirstChildElement("Passwords");
        if (!root) {
            swprintf_s(debugMsg, L"No 'Passwords' root element found in %s.", tempPath.c_str());
            MessageBoxW(nullptr, debugMsg, L"Error", MB_OK | MB_ICONERROR);
            //DeleteFileW(tempPath.c_str());
            SecureDeleteFile(tempPath.c_str());
            SecureZeroMemory(tempKey.data(), tempKey.size());
            tempKey.clear();
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
                entry.color = RGB(255, 255, 255); // Default color
            }
            const char* creationDate = entryElement->Attribute("creationDate");
            entry.creationDate = creationDate ? Utf8ToWstring(creationDate) : L"";
            entries.push_back(entry);
        }

        // Clean up
        //DeleteFileW(tempPath.c_str());
        SecureDeleteFile(tempPath.c_str());
        SecureZeroMemory(tempKey.data(), tempKey.size());
        tempKey.clear();
        swprintf_s(debugMsg, L"Loaded %zu entries from XML.", entries.size());
        PopulateListView();
        return true;
    }
    else {
        MessageBoxW(nullptr, L"Encryption key not available.", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }
}


bool SecureDeleteFile(const std::wstring& filePath) {
    HANDLE hFile = CreateFileW(filePath.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    LARGE_INTEGER fileSize;
    GetFileSizeEx(hFile, &fileSize);
    DWORD fileSizeLow = fileSize.LowPart;
    BYTE* buffer = new BYTE[fileSizeLow];
    if (buffer) {
        // Fill with random data
        for (DWORD i = 0; i < fileSizeLow; i++) buffer[i] = rand() % 256;
        DWORD bytesWritten;
        SetFilePointer(hFile, 0, nullptr, FILE_BEGIN);
        WriteFile(hFile, buffer, fileSizeLow, &bytesWritten, nullptr);
        delete[] buffer;
    }

    CloseHandle(hFile);
    return DeleteFileW(filePath.c_str()) != 0;
   
}


void SaveXML()
{
    if (resto == 1) {
        return; // Skip saving during backup restoration
    }

    std::wstring xmlPath = GetFullFilePath(L"data.xml");
    std::wstring tempPath = GetFullFilePath(L"temp_plain.xml");

    // Generate backup filename with current date
    SYSTEMTIME st;
    GetSystemTime(&st);
    WCHAR backupFileName[32];
    swprintf_s(backupFileName, L"Backup-%d-%d-%d.xml", st.wMonth, st.wDay, st.wYear % 100);
    std::wstring backupPath = GetFullFilePath(backupFileName);

    // Create a backup if it doesn't exist
    if (PathFileExistsW(xmlPath.c_str()) && !PathFileExistsW(backupPath.c_str())) {
        if (!CopyFileW(xmlPath.c_str(), backupPath.c_str(), FALSE)) {
            // Non-critical, proceed with saving
            // MessageBoxW(nullptr, L"Warning: Failed to create backup.", L"Backup Warning", MB_OK | MB_ICONWARNING);
        }
    }

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLDeclaration* decl = doc.NewDeclaration();
    doc.InsertFirstChild(decl);

    tinyxml2::XMLElement* root = doc.NewElement("Passwords");
    doc.InsertEndChild(root);

    for (const auto& entry : entries) {
        tinyxml2::XMLElement* entryElement = doc.NewElement("Entry");

        auto createTextElement = [&](const char* elementName, const std::wstring& text) {
            tinyxml2::XMLElement* elem = doc.NewElement(elementName);
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
        createTextElement("creationDate", entry.creationDate);

        tinyxml2::XMLElement* colorElement = doc.NewElement("Color");
        colorElement->SetAttribute("R", GetRValue(entry.color));
        colorElement->SetAttribute("G", GetGValue(entry.color));
        colorElement->SetAttribute("B", GetBValue(entry.color));
        entryElement->InsertEndChild(colorElement);

        entryElement->SetAttribute("creationDate", StringToUTF8(entry.creationDate).c_str());

        root->InsertEndChild(entryElement);
    }

    // Save to temp plain file
    std::string tempPathUtf8 = WstringToUtf8(tempPath);
    if (doc.SaveFile(tempPathUtf8.c_str()) != tinyxml2::XML_SUCCESS) {
        MessageBoxW(nullptr, L"Failed to save temporary XML data. Data not saved.", L"Save Error", MB_OK | MB_ICONERROR);
       // DeleteFileW(tempPath.c_str());
          SecureDeleteFile(tempPath.c_str());
        return;
    }

    // Decrypt g_userKeyForXml
    std::vector<BYTE> tempKey = g_userKeyForXml;
    if (!tempKey.empty()) {
        DWORD cbData = static_cast<DWORD>(tempKey.size());
        if (!CryptUnprotectMemory(tempKey.data(), cbData, CRYPTPROTECTMEMORY_SAME_PROCESS)) {
            MessageBoxW(nullptr, L"Failed to decrypt key in memory.", L"Encryption Error", MB_OK | MB_ICONERROR);
            //DeleteFileW(tempPath.c_str());
            SecureDeleteFile(tempPath.c_str());
            SecureZeroMemory(tempKey.data(), tempKey.size());
            tempKey.clear();
            return;
        }

        // Encrypt temp file to final data.xml
        if (!EncryptFile(tempPath, xmlPath, std::wstring(tempKey.begin(), tempKey.end()))) {
            MessageBoxW(nullptr, L"Failed to encrypt XML data. Data not saved.", L"Encryption Error", MB_OK | MB_ICONERROR);
            //DeleteFileW(tempPath.c_str());
            SecureDeleteFile(tempPath.c_str());
            SecureZeroMemory(tempKey.data(), tempKey.size());
            tempKey.clear();
            return;
        }

        // Clean up tempKey
        SecureZeroMemory(tempKey.data(), tempKey.size());
        tempKey.clear();
    }
    else {
       // MessageBoxW(nullptr, L"User key not available for encryption. Data not saved.", L"Key Error", MB_OK | MB_ICONERROR);
        //DeleteFileW(tempPath.c_str());
        SecureDeleteFile(tempPath.c_str());
        return;
    }

    // Clean up temp plain file
    //DeleteFileW(tempPath.c_str());
    SecureDeleteFile(tempPath.c_str());
}



// Timer callback to clear clipboard
VOID CALLBACK ClearClipboardTimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    if (OpenClipboard(hWnd)) {
        EmptyClipboard();
        CloseClipboard();
    }
    KillTimer(hWnd, idEvent);
    g_clipboardTimer = 0;
}

void CopyToClipboard(HWND hWnd, const std::wstring& text) {
    if (OpenClipboard(hWnd)) {
        EmptyClipboard();

        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (text.size() + 1) * sizeof(wchar_t));
        if (hMem) {
            wchar_t* pMem = (wchar_t*)GlobalLock(hMem);
            if (pMem) {
                wcscpy_s(pMem, text.size() + 1, text.c_str());
                GlobalUnlock(hMem);

                SetClipboardData(CF_UNICODETEXT, hMem);

                if (g_clipboardTimer) {
                    KillTimer(hWnd, g_clipboardTimer);
                    g_clipboardTimer = 0;
                }

                g_clipboardTimer = SetTimer(hWnd, CLIPBOARD_CLEAR_TIMER, 30000, ClearClipboardTimerProc);
                if (!g_clipboardTimer) {
                    MessageBoxW(hWnd, L"Failed to set clipboard clear timer.", L"Warning", MB_OK | MB_ICONWARNING);
                }
            }
            else {
                GlobalFree(hMem);
            }
        }
        CloseClipboard();
        StartClipboardClearTimer(hWnd); // Clear after 15s
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
        lvi.mask = LVIF_TEXT | LVIF_PARAM; // Add LVIF_PARAM
        lvi.iItem = (int)i;
        lvi.pszText = (LPWSTR)entries[i].name.c_str();
        lvi.lParam = (LPARAM)i; // Set index as lParam
        ListView_InsertItem(hListView, &lvi);
    }
}





// Dialog procedure for the new password dialog
INT_PTR CALLBACK PasswordDialogProcNew(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    static HBRUSH hEditBrush = NULL; // Static brush for edit control background

    switch (message)
    {

    case WM_INPUTLANGCHANGE:
        // Prevent screenshots
        SetWindowDisplayAffinity(hDlg, WDA_EXCLUDEFROMCAPTURE);
        return TRUE;

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
            return 2;
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

            // Validate input to prevent injection
            if (!IsValidInput(g_enteredPassword))
            {
                MessageBoxW(hWnd, L"Password contains invalid characters (<, >, &, \", '). Please use a different password.", L"Error", MB_OK | MB_ICONERROR);
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
                    g_userKeyForXml = key;

                    // Pad g_userKeyForXml if needed
                    size_t blockSize = CRYPTPROTECTMEMORY_BLOCK_SIZE; // Typically 16
                    size_t keySize = g_userKeyForXml.size();
                    size_t paddedSize = ((keySize + blockSize - 1) / blockSize) * blockSize;
                    if (keySize < paddedSize) {
                        g_userKeyForXml.resize(paddedSize, 0); // Pad with zeros
                    }

                    // Encrypt g_userKeyForXml
                    if (!g_userKeyForXml.empty()) {
                        DWORD cbData = static_cast<DWORD>(g_userKeyForXml.size());
                        if (!CryptProtectMemory(g_userKeyForXml.data(), cbData, CRYPTPROTECTMEMORY_SAME_PROCESS)) {
                            MessageBoxW(hWnd, L"Failed to encrypt key in memory.", L"Error", MB_OK | MB_ICONERROR);
                            SecureZeroMemory(g_userKeyForXml.data(), g_userKeyForXml.size());
                            g_userKeyForXml.clear();
                            SecureZeroMemory(&tempPassword[0], tempPassword.size() * sizeof(wchar_t));
                            return 0;
                        }
                    }

                    std::ofstream outFile(pgpPath, std::ios::binary);
                    if (outFile.is_open())
                    {
                        // Write salt followed by key to password.pgp
                        outFile.write(reinterpret_cast<const char*>(salt.data()), salt.size());
                        outFile.write(reinterpret_cast<const char*>(key.data()), key.size());
                        outFile.close();
                        g_isPinValidated = true;
                        SecureZeroMemory(&g_enteredPassword[0], g_enteredPassword.size() * sizeof(wchar_t));
                        g_enteredPassword.clear();

                        if (resto == 0) {
                            if (!LoadXML(GetFullFilePath(L"data.xml"))) {
                                MessageBoxW(hWnd, L"Failed to initialize data file.", L"Error", MB_OK | MB_ICONERROR);
                                SecureZeroMemory(&tempPassword[0], tempPassword.size() * sizeof(wchar_t));
                                return 0;
                            }
                        }
                        else {
                            if (!LoadXML(GetFullFilePath(files.c_str()))) {
                                MessageBoxW(hWnd, L"Failed to load specified file.", L"Error", MB_OK | MB_ICONERROR);
                                SecureZeroMemory(&tempPassword[0], tempPassword.size() * sizeof(wchar_t));
                                return 0;
                            }
                        }

                        SecureZeroMemory(&tempPassword[0], tempPassword.size() * sizeof(wchar_t));
                        tempPassword.clear();
                        return 1;
                    }
                    else
                    {
                        MessageBoxW(hWnd, L"Failed to create password file.", L"Error", MB_OK | MB_ICONERROR);
                        SecureZeroMemory(g_userKeyForXml.data(), g_userKeyForXml.size());
                        g_userKeyForXml.clear();
                        SecureZeroMemory(&tempPassword[0], tempPassword.size() * sizeof(wchar_t));
                        return 0;
                    }
                }
                catch (const std::exception& e)
                {
                    WCHAR debugMsg[512];
                    swprintf_s(debugMsg, L"Error generating key: %S", e.what());
                    MessageBoxW(hWnd, debugMsg, L"Error", MB_OK | MB_ICONERROR);
                    SecureZeroMemory(g_userKeyForXml.data(), g_userKeyForXml.size());
                    g_userKeyForXml.clear();
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
                        SecureZeroMemory(&tempPassword[0], tempPassword.size() * sizeof(wchar_t));
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

                            // Pad g_userKeyForXml if needed
                            size_t blockSize = CRYPTPROTECTMEMORY_BLOCK_SIZE; // Typically 16
                            size_t keySize = g_userKeyForXml.size();
                            size_t paddedSize = ((keySize + blockSize - 1) / blockSize) * blockSize;
                            if (keySize < paddedSize) {
                                g_userKeyForXml.resize(paddedSize, 0); // Pad with zeros
                            }

                            // Encrypt g_userKeyForXml
                            if (!g_userKeyForXml.empty()) {
                                DWORD cbData = static_cast<DWORD>(g_userKeyForXml.size());
                                if (!CryptProtectMemory(g_userKeyForXml.data(), cbData, CRYPTPROTECTMEMORY_SAME_PROCESS)) {
                                    MessageBoxW(hWnd, L"Failed to encrypt key in memory.", L"Error", MB_OK | MB_ICONERROR);
                                    SecureZeroMemory(g_userKeyForXml.data(), g_userKeyForXml.size());
                                    g_userKeyForXml.clear();
                                    SecureZeroMemory(&tempPassword[0], tempPassword.size() * sizeof(wchar_t));
                                    return 0;
                                }
                            }

                            g_isPinValidated = true;
                            SecureZeroMemory(&g_enteredPassword[0], g_enteredPassword.size() * sizeof(wchar_t));
                            g_enteredPassword.clear();
                            if (!LoadXML(GetFullFilePath(L"data.xml")))
                            {
                                MessageBoxW(hWnd, L"Failed to load XML file.", L"Error", MB_OK | MB_ICONERROR);
                                SecureZeroMemory(g_userKeyForXml.data(), g_userKeyForXml.size());
                                g_userKeyForXml.clear();
                                SecureZeroMemory(&tempPassword[0], tempPassword.size() * sizeof(wchar_t));
                                return 1; // Maintain existing behavior
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
                        SecureZeroMemory(g_userKeyForXml.data(), g_userKeyForXml.size());
                        g_userKeyForXml.clear();
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



void SetWindowTextColor(HWND hWnd, COLORREF color) {
    HDC hdc = GetDC(hWnd); // Get the device context for the window
    if (hdc) {
        SetTextColor(hdc, color); // Set the text color
        ReleaseDC(hWnd, hdc); // Release the device context
    }
}



INT_PTR CALLBACK AboutNewDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HBITMAP hBitmap = nullptr;

    switch (message)
    {
    case WM_INITDIALOG:
    {
        // Prevent screenshots
        //SetWindowDisplayAffinity(hDlg, WDA_EXCLUDEFROMCAPTURE);





        return (INT_PTR)TRUE;
    }




    case WM_CTLCOLORSTATIC:
    {
        HDC hdcStatic = (HDC)wParam;
        extern bool isDarkTheme;
        extern HBRUSH hDarkGreyBrush;

        SetTextColor(hdcStatic, textColor);
        SetBkMode(hdcStatic, TRANSPARENT);
        return (INT_PTR)hDarkGreyBrush;

        break;
    }

    case WM_COMMAND:
    {
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }

    case WM_DESTROY:
    {


    }
    return (INT_PTR)FALSE;
    }
}


bool OutputDebugStringCheck() {
    BOOL remote = FALSE;
    CheckRemoteDebuggerPresent(GetCurrentProcess(), &remote);
    return IsDebuggerPresent() || remote;
}


// Add more anti-debugging techniques
bool IsBeingDebugged() {
    BOOL remoteDebugger = FALSE;
    CheckRemoteDebuggerPresent(GetCurrentProcess(), &remoteDebugger);
    return IsDebuggerPresent() || remoteDebugger;
}


// Check for debugger
bool IsDebuggerAttached() {
    BOOL remoteDebuggerPresent = FALSE;
    CheckRemoteDebuggerPresent(GetCurrentProcess(), &remoteDebuggerPresent);
    return IsDebuggerPresent() || remoteDebuggerPresent;
}

// Check for popular VM processes
bool IsRunningInVM() {
     

    const wchar_t* guestOnlyProcs[] = {
        L"vmtoolsd.exe",
        L"vboxservice.exe",
        L"qemu-ga.exe"
    };

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return false;

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(pe);
    if (Process32FirstW(hSnapshot, &pe)) {
        do {
            for (const auto& proc : guestOnlyProcs) {
                if (_wcsicmp(proc, pe.szExeFile) == 0) {
                    CloseHandle(hSnapshot);
                    return true;
                }
            }
        } while (Process32NextW(hSnapshot, &pe));
    }
    CloseHandle(hSnapshot);
    return false;
}

// Check for VM hardware via CPUID
/*
bool IsVMCPU() {
    int cpuInfo[4] = { 0 };
    __cpuid(cpuInfo, 1);
    return (cpuInfo[2] >> 31) & 1; // Hypervisor bit
}
*/

/*
bool CheckPEBBeingDebuggedFlag() {
    // Works only on 64-bit Windows
    PBYTE peb = (PBYTE)__readgsqword(0x60); // GS:[0x60] = PEB
    return peb[2] != 0; // BeingDebugged is byte at offset 2
}
*/


bool IsVMCPU() {
    int cpuInfo[4] = { 0 };
    __cpuid(cpuInfo, 1);
    if (!((cpuInfo[2] >> 31) & 1)) return false; // Not under hypervisor

    char hyperVendor[13] = {};
    __cpuid(cpuInfo, 0x40000000); // Hypervisor vendor string
    memcpy(&hyperVendor[0], &cpuInfo[1], 4); // EBX
    memcpy(&hyperVendor[4], &cpuInfo[2], 4); // ECX
    memcpy(&hyperVendor[8], &cpuInfo[3], 4); // EDX

    return (
        strcmp(hyperVendor, "Microsoft Hv") == 0 ||
        strcmp(hyperVendor, "VMwareVMware") == 0 ||
        strcmp(hyperVendor, "KVMKVMKVM") == 0 ||
        strcmp(hyperVendor, "XenVMMXenVMM") == 0
        );
}

// Define the function pointer type for NtQueryInformationProcess
typedef NTSTATUS(NTAPI* pNtQueryInformationProcess)(
    HANDLE ProcessHandle,
    PROCESSINFOCLASS ProcessInformationClass,
    PVOID ProcessInformation,
    ULONG ProcessInformationLength,
    PULONG ReturnLength
    );




bool NtQueryDebugFlag()
{
    // --- resolve NtQueryInformationProcess once ---
    using pNtQIP = NTSTATUS(NTAPI*)(
        HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);

    static pNtQIP NtQIP = reinterpret_cast<pNtQIP>(
        GetProcAddress(GetModuleHandleW(L"ntdll.dll"),
            "NtQueryInformationProcess"));

    if (!NtQIP)
        return false;

    PROCESS_BASIC_INFORMATION pbi{};
    if (NtQIP(GetCurrentProcess(),
        ProcessBasicInformation,
        &pbi, sizeof pbi, nullptr) != 0)
        return false;

    auto peb = static_cast<PBYTE>(pbi.PebBaseAddress);
    bool beingDebugged = peb[2] != 0;          // PEB->BeingDebugged

    // Secondary cross‑check:
    bool apiSays = IsDebuggerPresent();

    std::printf("PEB.BeingDebugged = %d  IsDebuggerPresent = %d\n",
        beingDebugged, apiSays);

    return beingDebugged;
}



bool CheckForBreakpointsInCode() {
    MEMORY_BASIC_INFORMATION mbi = {};
    BYTE* codeBase = (BYTE*)CheckForBreakpointsInCode; // start near this function
    SIZE_T checkSize = 512; // check first 512 bytes of current function

    VirtualQuery(codeBase, &mbi, sizeof(mbi));
    if (mbi.State != MEM_COMMIT || !(mbi.Protect & PAGE_EXECUTE_READ)) return false;

    for (SIZE_T i = 0; i < checkSize; ++i) {
        if (codeBase[i] == 0xCC) return true; // 0xCC = INT3
    }
    return false;
}

bool IsValidInput(const std::wstring& input) {
    // Prevent XML/HTML injection
    return input.find_first_of(L"<>&\"'") == std::wstring::npos;
}



////////////////////////////

bool ComputeFileHash(const std::wstring& filePath, std::vector<BYTE>& hash) {
    BCRYPT_ALG_HANDLE hAlg = nullptr;
    BCRYPT_HASH_HANDLE hHash = nullptr;
    DWORD hashLength = 32; // SHA-256 produces 32 bytes
    hash.resize(hashLength);
    bool success = false;
    HANDLE hFile = INVALID_HANDLE_VALUE;

    // Open the algorithm provider
    if (!BCRYPT_SUCCESS(BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, 0))) {
        goto Cleanup;
    }

    // Create a hash object
    if (!BCRYPT_SUCCESS(BCryptCreateHash(hAlg, &hHash, nullptr, 0, nullptr, 0, 0))) {
        goto Cleanup;
    }

    // Open the file
    hFile = CreateFileW(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        goto Cleanup;
    }

    // Read and hash the file in chunks
    BYTE buffer[4096];
    DWORD bytesRead;
    while (ReadFile(hFile, buffer, sizeof(buffer), &bytesRead, nullptr) && bytesRead > 0) {
        if (!BCRYPT_SUCCESS(BCryptHashData(hHash, buffer, bytesRead, 0))) {
            goto Cleanup;
        }
    }

    // Finalize the hash
    if (!BCRYPT_SUCCESS(BCryptFinishHash(hHash, hash.data(), hashLength, 0))) {
        goto Cleanup;
    }

    success = true;

Cleanup:
    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
    }
    if (hHash) {
        BCryptDestroyHash(hHash);
    }
    if (hAlg) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
    }
    return success;
}

bool VerifyAuthenticode(const std::wstring& exePath) {
    bool signaturePresent = false;
    HCERTSTORE hStore = nullptr;
    HCRYPTMSG hMsg = nullptr;
    PCCERT_CONTEXT pCertContext = nullptr;

    // Open the file for signature verification
    DWORD dwEncoding, dwContentType, dwFormatType;
    if (!CryptQueryObject(CERT_QUERY_OBJECT_FILE,
        exePath.c_str(),
        CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED,
        CERT_QUERY_FORMAT_FLAG_BINARY,
        0,
        &dwEncoding,
        &dwContentType,
        &dwFormatType,
        &hStore,
        &hMsg,
        nullptr)) {
        goto Cleanup;
    }

    // Check if a message (signature) exists
    if (hMsg) {
        DWORD cbSigners = 0;
        if (!CryptMsgGetParam(hMsg, CMSG_SIGNER_COUNT_PARAM, 0, nullptr, &cbSigners) || cbSigners == 0) {
            goto Cleanup;
        }

        // Get the signer information
        PCMSG_SIGNER_INFO pSignerInfo = nullptr;
        DWORD cbSignerInfo = 0;
        if (!CryptMsgGetParam(hMsg, CMSG_SIGNER_INFO_PARAM, 0, nullptr, &cbSignerInfo) ||
            !(pSignerInfo = (PCMSG_SIGNER_INFO)LocalAlloc(LMEM_FIXED, cbSignerInfo)) ||
            !CryptMsgGetParam(hMsg, CMSG_SIGNER_INFO_PARAM, 0, pSignerInfo, &cbSignerInfo)) {
            goto Cleanup;
        }

        // A signer info exists, indicating a signature is present
        signaturePresent = true;

        LocalFree(pSignerInfo);
    }

Cleanup:
    if (pCertContext) CertFreeCertificateContext(pCertContext);
    if (hMsg) CryptMsgClose(hMsg);
    if (hStore) CertCloseStore(hStore, 0);

    return signaturePresent;
}



bool VerifyHash(const BYTE* hash, DWORD hashSize, const BYTE* expectedHash, DWORD expectedHashSize) {
    if (hashSize != expectedHashSize) {
        return false;
    }
    return memcmp(hash, expectedHash, hashSize) == 0;
}


bool ReadHashFromFile(const std::wstring& hashFilePath, std::vector<BYTE>& hash) {
    std::ifstream inFile(hashFilePath);
    if (!inFile.is_open()) {
        return false;
    }

    std::string line;
    std::string hashStr;
    int lineCount = 0;

    // Read lines, expecting header (Hash, ----) followed by hash
    while (std::getline(inFile, line)) {
        lineCount++;
        if (lineCount <= 2) {
            // Skip header lines (Hash and ----)
            continue;
        }
        // Remove whitespace
        line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());
        if (!line.empty()) {
            hashStr = line;
            break;
        }
    }
    inFile.close();

    // Validate hash string (64 chars for SHA-256)
    if (hashStr.length() != 64) {
        return false;
    }

    // Convert hex string to bytes
    hash.resize(32); // SHA-256 is 32 bytes
    try {
        for (size_t i = 0; i < hashStr.length(); i += 2) {
            std::string byteStr = hashStr.substr(i, 2);
            hash[i / 2] = static_cast<BYTE>(std::stoul(byteStr, nullptr, 16));
        }
    }
    catch (const std::exception&) {
        hash.clear();
        return false;
    }

    return true;
}

bool VerifyExecutableIntegrity()
{
    WCHAR path[MAX_PATH];
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    std::wstring exePath = path;

    // First: Verify Authenticode signature
    if (!VerifyAuthenticode(exePath)) {
        
        
      // return false; //no cert found
    }

    // Fallback: Verify SHA-256 hash
    std::vector<BYTE> computedHash;
    if (!ComputeFileHash(exePath, computedHash)) {
      //  return false; // Failed to compute hash
    }

    // Read expected hash from hash.256
    std::vector<BYTE> expectedHash;
    if (!ReadHashFromFile(GetFullFilePath(L"hash.256"), expectedHash)) {
        return false; // Failed to read or parse hash file
    }

    return VerifyHash(computedHash.data(), static_cast<DWORD>(computedHash.size()),
        expectedHash.data(), static_cast<DWORD>(expectedHash.size()));
}

//signtool sign /f your_certificate.pfx /p password /t http://timestamp.digicert.com pwrd.exe
////use  Get-FileHash -Path .\pwrd.exe -Algorithm SHA256

 



 