//
// C++ (Windows 7+)
// Traymond (Fork by dkxce) at: https://github.com/dkxce
// Original at: https://github.com/fcFn/traymond
// en,ru,1251,utf-8
//

#include <Windows.h>
#include <windowsx.h>
#include <string>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <thread>

#define VK_Z_KEY 0x5A
// These keys are used to send windows to tray
#define TRAY_KEY VK_Z_KEY
#define MOD_KEY MOD_WIN + MOD_SHIFT

#define WM_ICON 0x1C0A
#define WM_OURICON 0x1C0B
#define EXIT_ID 0x99
#define SHOW_ALL_ID 0x98
#define SHOW_HOMEPAGE 0x97
#define HIDE_FOREGROUND 0x96
#define MNIT_AUTORUN 0x95
#define MNIT_MLTICNS 0x954
#define FIRST_MENU 0xA0
#define MAXIMUM_WINDOWS 100


using namespace std;

bool multiIcons = true;
bool initializated = false;
WORD itmCounter = 0;
std::vector<MENUITEMINFO> trayedMenu;

// Stores hidden window record.
typedef struct HIDDEN_WINDOW {
    NOTIFYICONDATA icon;
    HWND window;
} HIDDEN_WINDOW;

// Current execution context
typedef struct TRCONTEXT {
    HWND mainWindow;
    HIDDEN_WINDOW icons[MAXIMUM_WINDOWS];
    HMENU trayMenu;
    int iconIndex; // How many windows are currently hidden
} TRCONTEXT;

HANDLE saveFile;

// Saves our hidden windows so they can be restored in case of crashing.
void save(const TRCONTEXT* context) {
    DWORD numbytes;
    // Truncate file
    SetFilePointer(saveFile, 0, NULL, FILE_BEGIN);
    SetEndOfFile(saveFile);
    if (!context->iconIndex) {
        return;
    }
    for (int i = 0; i < context->iconIndex; i++)
    {
        if (context->icons[i].window) {
            std::string str;
            str = std::to_string((long)context->icons[i].window);
            str += ',';
            const char* handleString = str.c_str();
            WriteFile(saveFile, handleString, strlen(handleString), &numbytes, NULL);
        }
    }
}

// Get Executable File Path
std::string getExecutablePath() {
    char rawPathName[MAX_PATH];
    GetModuleFileNameA(NULL, rawPathName, MAX_PATH);
    return std::string(rawPathName);
}

// Set Menu Item Checked or Not
void setMenuItemChecked(TRCONTEXT* context, UINT wID, bool checked)
{
    int pos;                                          // use signed int so we can count down and detect passing 0
    MENUITEMINFO mf;

    ZeroMemory(&mf, sizeof(mf));                      // request just item ID
    mf.cbSize = sizeof(mf);
    mf.fMask = MIIM_ID | MIIM_STATE;
    for (pos = GetMenuItemCount(context->trayMenu); --pos >= 0; )         // enumerate menu items
        if (GetMenuItemInfo(context->trayMenu, (UINT)pos, TRUE, &mf))    // if we find the ID we are looking for return TRUE
            if (mf.wID == wID)
            {
                //BOOL checked = GetMenuState(context->trayMenu, (UINT)pos, MF_BYPOSITION) & MF_CHECKED;
                CheckMenuItem(context->trayMenu, (UINT)pos, MF_BYPOSITION | (checked ? MF_CHECKED : MF_UNCHECKED));
                //DrawMenuBar((HWND)context->trayMenu);
            };
}

// Set Application Autorun State
bool toggleAutorun(TRCONTEXT* context, bool toggle)
{
    DWORD buffSize = 1024;
    char buff[1024];
    if (RegGetValue(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", "Traymond", RRF_RT_REG_SZ, nullptr, buff, &buffSize) == ERROR_SUCCESS)
    {
        if (toggle)
        {
            HKEY hkey = NULL;
            LONG createStatus = RegCreateKey(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", &hkey);
            RegDeleteValue(hkey, "Traymond");
            RegCloseKey(hkey);
            //MessageBox(0, "Traymond successfully removed from autorun", "Traymond", 0);
            setMenuItemChecked(context, MNIT_AUTORUN, false);
            return false;
        };
        return true;
    }
    else
    {
        if (toggle)
        {
            std::string currPath = getExecutablePath();
            HKEY hkey = NULL;
            LONG createStatus = RegCreateKey(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", &hkey);
            LONG status = RegSetValueEx(hkey, "Traymond", 0, REG_SZ, (BYTE*)currPath.c_str(), (currPath.size() + 1) * sizeof(wchar_t));
            RegCloseKey(hkey);
            //MessageBox(0, "Traymond successfully added to autorun", "Traymond", 0);
            setMenuItemChecked(context, MNIT_AUTORUN, true);
            return true;
        };
        return false;
    };
}

// Toggles Muiltiple Tray Icons
bool toggleMultiIcons(TRCONTEXT* context, bool toggle)
{

    DWORD buffSize = 1024;
    char buff[1024] = "false";
    RegGetValue(HKEY_CURRENT_USER, "SOFTWARE\\Traymond", "DisableMultiIcons", RRF_RT_REG_SZ, nullptr, buff, &buffSize);
    if (strcmp(buff, "true") == 0)
    {
        if (toggle)
        {
            HKEY hkey = NULL;
            LONG createStatus = RegCreateKey(HKEY_CURRENT_USER, "SOFTWARE\\Traymond", &hkey);
            RegDeleteValue(hkey, "DisableMultiIcons");
            RegCloseKey(hkey);
            setMenuItemChecked(context, MNIT_MLTICNS, false);
            //
            if (context->iconIndex > 0)
                for (int i = 0; i < context->iconIndex; i++)
                {
                    Shell_NotifyIcon(NIM_ADD, &context->icons[i].icon);
                    Shell_NotifyIcon(NIM_SETVERSION, &context->icons[i].icon);
                };
            //
            multiIcons = true;
            return false;
        };
        multiIcons = false;
        return true;
    }
    else
    {
        if (toggle)
        {
            std::string currPath = "true";
            HKEY hkey = NULL;
            LONG createStatus = RegCreateKey(HKEY_CURRENT_USER, "SOFTWARE\\Traymond", &hkey);
            LONG status = RegSetValueEx(hkey, "DisableMultiIcons", 0, REG_SZ, (BYTE*)currPath.c_str(), (currPath.size() + 1) * sizeof(wchar_t));
            RegCloseKey(hkey);
            setMenuItemChecked(context, MNIT_MLTICNS, true);
            //
            if(context->iconIndex > 0)
                for (int i = 0; i < context->iconIndex; i++)
                    Shell_NotifyIcon(NIM_DELETE, &context->icons[i].icon);
            //
            multiIcons = false;
            return true;
        };
        multiIcons = true;
        return false;
    };
}

// Replace Colors in Bitmap
HBITMAP replaceColor(HBITMAP hBmp, COLORREF cOldColor, COLORREF cNewColor)
{
    HBITMAP RetBmp = NULL;
    if (hBmp)
    {
        HDC BufferDC = CreateCompatibleDC(NULL);
        if (BufferDC)
        {
            HGDIOBJ PreviousBufferObject = SelectObject(BufferDC, hBmp);
            HDC DirectDC = CreateCompatibleDC(NULL);
            if (DirectDC)
            {
                BITMAP bm;
                GetObject(hBmp, sizeof(bm), &bm);

                BITMAPINFO RGB32BitsBITMAPINFO;
                ZeroMemory(&RGB32BitsBITMAPINFO, sizeof(BITMAPINFO));
                RGB32BitsBITMAPINFO.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                RGB32BitsBITMAPINFO.bmiHeader.biWidth = bm.bmWidth;
                RGB32BitsBITMAPINFO.bmiHeader.biHeight = bm.bmHeight;
                RGB32BitsBITMAPINFO.bmiHeader.biPlanes = 1;
                RGB32BitsBITMAPINFO.bmiHeader.biBitCount = 32;

                UINT* ptPixels;
                HBITMAP DirectBitmap = CreateDIBSection(DirectDC, (BITMAPINFO*)&RGB32BitsBITMAPINFO, DIB_RGB_COLORS, (void**)&ptPixels, NULL, 0);
                if (DirectBitmap)
                {
                    HGDIOBJ PreviousObject = SelectObject(DirectDC, DirectBitmap);
                    BitBlt(DirectDC, 0, 0, bm.bmWidth, bm.bmHeight, BufferDC, 0, 0, SRCCOPY);
                    for (int i = ((bm.bmWidth * bm.bmHeight) - 1); i >= 0; i--) if (ptPixels[i] == cOldColor) ptPixels[i] = cNewColor;
                    SelectObject(DirectDC, PreviousObject);
                    RetBmp = DirectBitmap;
                };
                DeleteDC(DirectDC);
            };
            SelectObject(BufferDC, PreviousBufferObject);
            DeleteDC(BufferDC);
        };
    };
    return RetBmp;
}

// Resize Bitmap to 16x16
HBITMAP resizeBitmap(HBITMAP hbmpSrc)
{
    HDC hdcScreen = GetDC(0);
    HBITMAP hbmpDst = 0;
    {
        HDC hdcSrc = CreateCompatibleDC(hdcScreen);
        HGDIOBJ gdiSrc = SelectObject(hdcSrc, hbmpSrc);
        BITMAP bmpSrc = { 0 };
        GetObject(hbmpSrc, sizeof(BITMAP), &bmpSrc);
        //if (bmpSrc.bmWidth > 16 && bmpSrc.bmHeight > 16)
        {
            HDC hdcDst = CreateCompatibleDC(hdcScreen);
            hbmpDst = CreateCompatibleBitmap(hdcScreen, 16, 16);
            HGDIOBJ gdiDsrt = SelectObject(hdcDst, hbmpDst);
            StretchBlt(hdcDst, 0, 0, 16, 16, hdcSrc, 0, 0, bmpSrc.bmWidth, bmpSrc.bmHeight, SRCCOPY);
            SelectObject(hdcDst, gdiDsrt);
            SelectObject(hdcSrc, gdiSrc);
            DeleteDC(hdcDst);
        };
        DeleteDC(hdcSrc);
    };
    DeleteDC(hdcScreen);
    return replaceColor(hbmpDst == 0 ? hbmpSrc : hbmpDst, RGB(0, 0, 0), RGB(255, 255, 255));
}

// Restore Window
void showWindow(TRCONTEXT* context, LPARAM lParam) 
{
    std::vector<HIDDEN_WINDOW> temp = std::vector<HIDDEN_WINDOW>();
    for (int i = 0; i < context->iconIndex; i++)
    {
        Shell_NotifyIcon(NIM_DELETE, &context->icons[i].icon);
        if (context->icons[i].icon.uID == HIWORD(lParam))
        {
            ShowWindow(context->icons[i].window, SW_SHOW);
            SetForegroundWindow(context->icons[i].window);            
        }
        else temp.push_back(context->icons[i]);
    };
    // strip
    if (temp.size() < context->iconIndex)
    {
        context->iconIndex = temp.size();
        if (temp.size() > 0)
        {
            memcpy_s(context->icons, sizeof(context->icons), &temp.front(), sizeof(HIDDEN_WINDOW) * temp.size());
            if(multiIcons)
                for (int i = 0; i < temp.size(); i++)
                {
                    Shell_NotifyIcon(NIM_ADD, &temp[i].icon);
                    Shell_NotifyIcon(NIM_SETVERSION, &temp[i].icon);
                };
        }; 
    };
    temp.clear();
    save(context);
}

// Add Window Menu Item
void addWindowMenuItem(TRCONTEXT* context, NOTIFYICONDATA nid)
{
    if (!initializated)
    {
        initializated = true;
        InsertMenu(context->trayMenu, 0, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);
    };

    ICONINFO iconinfo;
    GetIconInfo(nid.hIcon, &iconinfo);
    HBITMAP hBitmap = resizeBitmap(iconinfo.hbmColor);

    MENUITEMINFO currentItem;
    currentItem.cbSize = sizeof(MENUITEMINFO);
    currentItem.fMask = MIIM_STRING | MIIM_ID | MIIM_DATA | MIIM_BITMAP;
    currentItem.fType = MFT_STRING;
    currentItem.dwTypeData = nid.szTip;
    currentItem.cch = sizeof(nid.szTip);
    currentItem.dwItemData = nid.uID;
    currentItem.wID = 0xA0 + itmCounter++;
    currentItem.hbmpItem = hBitmap;
    InsertMenuItem(context->trayMenu, 0, FALSE, &currentItem);
    trayedMenu.push_back(currentItem);

    if (itmCounter >= 0xFF5F) itmCounter = 0; // reset counter
}

// Clear Menu
void clearMenu(TRCONTEXT* context)
{
    if (trayedMenu.size() == 0) return;
    for (int i = trayedMenu.size() - 1; i >= 0; i--)
        RemoveMenu(context->trayMenu, i, MF_BYPOSITION);
    trayedMenu.clear();

    if (context->iconIndex > 0)
        for (int i = 0; i < context->iconIndex; i++)
            addWindowMenuItem(context, context->icons[i].icon);
}

// Click on Menu Item
void clickWindowMenuItem(TRCONTEXT* context, UINT wID)
{
    if (trayedMenu.size() == 0) return;
    for (int i = trayedMenu.size() - 1; i >= 0; i--)
    {
        if (trayedMenu[i].wID == wID)
        {
            showWindow(context, trayedMenu[i].dwItemData << 16);
            trayedMenu.erase(std::next(trayedMenu.begin(), i));
            RemoveMenu(context->trayMenu, i, MF_BYPOSITION);
        };
    };
}

// Minimizes the current window to tray.
// Uses currently focused window unless supplied a handle as the argument.
void minimizeToTray(TRCONTEXT* context, long restoreWindow) {
    // Taskbar and desktop windows are restricted from hiding.
    const char restrictWins[][14] = { {"WorkerW"}, {"Shell_TrayWnd"} };

    HWND currWin = 0;
    if (!restoreWindow) {
        currWin = GetForegroundWindow();
    }
    else {
        currWin = reinterpret_cast<HWND>(restoreWindow);
    }

    if (!currWin) {
        return;
    }

    char className[256];
    if (!GetClassName(currWin, className, 256)) {
        return;
    }
    else {
        for (int i = 0; i < sizeof(restrictWins) / sizeof(*restrictWins); i++)
        {
            if (strcmp(restrictWins[i], className) == 0) {
                return;
            }
        }
    }
    if (context->iconIndex == MAXIMUM_WINDOWS) {
        MessageBox(NULL, "Error! Too many hidden windows. Please unhide some.", "Traymond", MB_OK | MB_ICONERROR);
        return;
    }
    ULONG_PTR icon = GetClassLongPtr(currWin, GCLP_HICONSM);
    if (!icon) {
        icon = SendMessage(currWin, WM_GETICON, 2, NULL);
        if (!icon) {
            return;
        }
    }
    
    // Check if already in Tray
    if(context->iconIndex > 0)
        for (int i = 0; i < context->iconIndex; i++)
            if (context->icons[i].icon.uID == LOWORD(reinterpret_cast<UINT>(currWin)))
            {
                ShowWindow(currWin, SW_HIDE);
                if (!restoreWindow) save(context);
                return;
            };

    NOTIFYICONDATA nid;
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = context->mainWindow;
    nid.hIcon = (HICON)icon;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_SHOWTIP;
    nid.uVersion = NOTIFYICON_VERSION_4;
    nid.uID = LOWORD(reinterpret_cast<UINT>(currWin));
    nid.uCallbackMessage = WM_ICON;
    GetWindowText(currWin, nid.szTip, 128);
    context->icons[context->iconIndex].icon = nid;
    context->icons[context->iconIndex].window = currWin;
    context->iconIndex++;
    if (multiIcons)
    {
        Shell_NotifyIcon(NIM_ADD, &nid);
        Shell_NotifyIcon(NIM_SETVERSION, &nid);
    };
    ShowWindow(currWin, SW_HIDE);
    if (!restoreWindow) save(context);
    addWindowMenuItem(context, nid);
}

// Adds our own icon to tray
void createTrayIcon(HWND mainWindow, HINSTANCE hInstance, NOTIFYICONDATA* icon) {
    icon->cbSize = sizeof(NOTIFYICONDATA);
    icon->hWnd = mainWindow;
    icon->hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(101));
    icon->uFlags = NIF_ICON | NIF_TIP | NIF_SHOWTIP | NIF_MESSAGE;
    icon->uVersion = NOTIFYICON_VERSION_4;
    icon->uID = reinterpret_cast<UINT>(mainWindow);
    icon->uCallbackMessage = WM_OURICON;
    strcpy_s(icon->szTip, "Traymond");
    Shell_NotifyIcon(NIM_ADD, icon);
    Shell_NotifyIcon(NIM_SETVERSION, icon);
}

// Creates our tray icon menu
void createTrayMenu(TRCONTEXT* context) {
    context->trayMenu = CreatePopupMenu();

    MENUITEMINFO exitMenuItem;
    exitMenuItem.cbSize = sizeof(MENUITEMINFO);
    exitMenuItem.fMask = MIIM_STRING | MIIM_ID;
    exitMenuItem.fType = MFT_STRING;
    exitMenuItem.dwTypeData = "Exit";
    exitMenuItem.cch = 5;
    exitMenuItem.wID = EXIT_ID;

    MENUITEMINFO showAllMenuItem;
    showAllMenuItem.cbSize = sizeof(MENUITEMINFO);
    showAllMenuItem.fMask = MIIM_STRING | MIIM_ID;
    showAllMenuItem.fType = MFT_STRING;
    showAllMenuItem.dwTypeData = "Restore All Windows";
    showAllMenuItem.cch = 20;
    showAllMenuItem.wID = SHOW_ALL_ID;

    MENUITEMINFO homePageItem;
    homePageItem.cbSize = sizeof(MENUITEMINFO);
    homePageItem.fMask = MIIM_STRING | MIIM_ID;
    homePageItem.fType = MFT_STRING;
    homePageItem.dwTypeData = "Help (Win+Shift+Z -> Tray)";
    homePageItem.cch = 27;
    homePageItem.wID = SHOW_HOMEPAGE;

    MENUITEMINFO hideFgndWndItem;
    hideFgndWndItem.cbSize = sizeof(MENUITEMINFO);
    hideFgndWndItem.fMask = MIIM_STRING | MIIM_ID;
    hideFgndWndItem.fType = MFT_STRING;
    hideFgndWndItem.dwTypeData = "Hide Foreground Window to Tray";
    hideFgndWndItem.cch = 31;
    hideFgndWndItem.wID = HIDE_FOREGROUND;

    MENUITEMINFO autoRunMenu;
    autoRunMenu.cbSize = sizeof(MENUITEMINFO);
    autoRunMenu.fMask = MIIM_STRING | MIIM_ID | MIIM_STATE;
    autoRunMenu.fType = MFT_STRING;
    autoRunMenu.dwTypeData = "Autorun at Windows Startup";
    autoRunMenu.fState = toggleAutorun(context, false) ? MFS_CHECKED : 0;
    autoRunMenu.cch = 27;
    autoRunMenu.wID = MNIT_AUTORUN;
    
    MENUITEMINFO nonMultiMenuItem;
    nonMultiMenuItem.cbSize = sizeof(MENUITEMINFO);
    nonMultiMenuItem.fMask = MIIM_STRING | MIIM_ID | MIIM_STATE;
    nonMultiMenuItem.fType = MFT_STRING;
    nonMultiMenuItem.dwTypeData = "Disable Tray MultiIcons";
    nonMultiMenuItem.fState = toggleMultiIcons(context, false) ? MFS_CHECKED : 0;
    nonMultiMenuItem.cch = 24;
    nonMultiMenuItem.wID = MNIT_MLTICNS;
   
    InsertMenuItem(context->trayMenu, 0, FALSE, &nonMultiMenuItem);
    InsertMenuItem(context->trayMenu, 0, FALSE, &autoRunMenu);
    InsertMenuItem(context->trayMenu, 0, FALSE, &homePageItem);
    InsertMenuItem(context->trayMenu, 0, FALSE, &exitMenuItem);
    InsertMenu(context->trayMenu, 0, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);
    InsertMenuItem(context->trayMenu, 0, FALSE, &showAllMenuItem);
    InsertMenuItem(context->trayMenu, 0, FALSE, &hideFgndWndItem);
}

// Shows all hidden windows;
void showAllWindows(TRCONTEXT* context) 
{
    for (int i = 0; i < context->iconIndex; i++)
    {
        ShowWindow(context->icons[i].window, SW_SHOW);
        Shell_NotifyIcon(NIM_DELETE, &context->icons[i].icon);
        context->icons[i] = {};
    };
    save(context);
    context->iconIndex = 0;
    clearMenu(context);
}

void exitApp() 
{
    PostQuitMessage(0);
}

// Creates and reads the save file to restore hidden windows in case of unexpected termination
void startup(TRCONTEXT* context) 
{
    if ((saveFile = CreateFile("traymond.dat", GENERIC_READ | GENERIC_WRITE, \
        0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE) {
        MessageBox(NULL, "Error! Traymond could not create a save file.", "Traymond", MB_OK | MB_ICONERROR);
        exitApp();
    };

    // Check if we've crashed (i. e. there is a save file) during current uptime and
    // if there are windows to restore, in which case restore them and
    // display a reassuring message.
    if (GetLastError() == ERROR_ALREADY_EXISTS) 
    {
        DWORD numbytes;
        DWORD fileSize = GetFileSize(saveFile, NULL);

        if (!fileSize) {
            return;
        };

        FILETIME saveFileWriteTime;
        GetFileTime(saveFile, NULL, NULL, &saveFileWriteTime);
        uint64_t writeTime = ((uint64_t)saveFileWriteTime.dwHighDateTime << 32 | (uint64_t)saveFileWriteTime.dwLowDateTime) / 10000;
        GetSystemTimeAsFileTime(&saveFileWriteTime);
        writeTime = (((uint64_t)saveFileWriteTime.dwHighDateTime << 32 | (uint64_t)saveFileWriteTime.dwLowDateTime) / 10000) - writeTime;

        if (GetTickCount64() < writeTime) {
            return;
        }

        std::vector<char> contents = std::vector<char>(fileSize);
        ReadFile(saveFile, &contents.front(), fileSize, &numbytes, NULL);
        char handle[10];
        int index = 0;
        for (size_t i = 0; i < fileSize; i++)
        {
            if (contents[i] != ',') {
                handle[index] = contents[i];
                index++;
            }
            else {
                index = 0;
                minimizeToTray(context, std::stoi(std::string(handle)));
                memset(handle, 0, sizeof(handle));
            }
        }
        std::string restore_message = "Traymond had previously been terminated unexpectedly.\n\nRestored " + \
            std::to_string(context->iconIndex) + (context->iconIndex > 1 ? " icons." : " icon.");
        MessageBox(NULL, restore_message.c_str(), "Traymond", MB_OK);
    }
}

static BOOL CALLBACK enumWindowCallback(HWND hWnd, LPARAM lparam) {
    int length = GetWindowTextLength(hWnd);
    char* buffer = new char[length + 1];
    GetWindowText(hWnd, buffer, length + 1);
    std::string windowTitle(buffer);
    delete[] buffer;

    if (IsWindowVisible(hWnd) && length != 0 && !(GetWindowLong(hWnd, GWL_EXSTYLE) & (WS_EX_TOOLWINDOW | WS_EX_TOPMOST)))
    {
        minimizeToTray((TRCONTEXT*)lparam, (long)hWnd);
        return FALSE;
    };
    return TRUE;
}

// Hide Next Foreground Window
void hideNextForegroundWindow(TRCONTEXT* context)
{
    EnumWindows(enumWindowCallback, (LPARAM)context);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    TRCONTEXT* context = reinterpret_cast<TRCONTEXT*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    POINT pt;
    switch (uMsg)
    {
        case WM_ICON:
            if (LOWORD(lParam) == WM_LBUTTONDBLCLK)
            {
                showWindow(context, lParam);
                clearMenu(context);
            };
            break;
        case WM_OURICON:
            if (LOWORD(lParam) == WM_RBUTTONUP)
            {
                SetForegroundWindow(hwnd);
                GetCursorPos(&pt);
                TrackPopupMenuEx(context->trayMenu, \
                    (GetSystemMetrics(SM_MENUDROPALIGNMENT) ? TPM_RIGHTALIGN : TPM_LEFTALIGN) | TPM_BOTTOMALIGN, \
                    pt.x, pt.y, hwnd, NULL);
            };
            break;
        case WM_COMMAND:
            if (HIWORD(wParam) == 0)
            {
                switch LOWORD(wParam)
                {   
                    case MNIT_MLTICNS:
                        toggleMultiIcons(context, true);
                        break;
                    case MNIT_AUTORUN:
                        toggleAutorun(context, true);
                        break;
                    case HIDE_FOREGROUND:                             
                        hideNextForegroundWindow(context);                  
                        break;
                    case SHOW_HOMEPAGE:                    
                        ShellExecuteA(0, 0, "explorer.exe", "http://github.com/dkxce/traymond", 0, SW_SHOWMAXIMIZED);                                     
                        break;
                    case SHOW_ALL_ID:
                        showAllWindows(context);
                        break;
                    case EXIT_ID:
                        exitApp();
                        break;
                };
                if (LOWORD(wParam) >= 0xA0)
                    clickWindowMenuItem(context, LOWORD(wParam));
            };
            break;
        case WM_HOTKEY: // We only have one hotkey, so no need to check the message
            minimizeToTray(context, NULL);
            break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

#pragma warning( push )
#pragma warning( disable : 4100 )
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
#pragma warning( pop )

    TRCONTEXT context = {};
    NOTIFYICONDATA icon = {};

    // Mutex to allow only one instance
    const char szUniqueNamedMutex[] = "traymond_mutex";
    HANDLE mutex = CreateMutex(NULL, TRUE, szUniqueNamedMutex);
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        MessageBox(NULL, "Error! Another instance of Traymond is already running.", "Traymond", MB_OK | MB_ICONERROR);
        return 1;
    }

    BOOL bRet;
    MSG msg;

    const char CLASS_NAME[] = "Traymond";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClass(&wc)) {
        return 1;
    }

    context.mainWindow = CreateWindow(CLASS_NAME, NULL, NULL, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);

    if (!context.mainWindow) {
        return 1;
    }

    // Store our context in main window for retrieval by WindowProc
    SetWindowLongPtr(context.mainWindow, GWLP_USERDATA, reinterpret_cast<LONG>(&context));

    if (!RegisterHotKey(context.mainWindow, 0, MOD_KEY | MOD_NOREPEAT, TRAY_KEY)) {
        MessageBox(NULL, "Error! Could not register the hotkey.", "Traymond", MB_OK | MB_ICONERROR);
        return 1;
    }

    createTrayIcon(context.mainWindow, hInstance, &icon);
    createTrayMenu(&context);    
    startup(&context);

    while ((bRet = GetMessage(&msg, 0, 0, 0)) != 0)
    {
        if (bRet != -1) {
            DispatchMessage(&msg);
        }
    }
    // Clean up on exit;
    showAllWindows(&context);
    Shell_NotifyIcon(NIM_DELETE, &icon);
    ReleaseMutex(mutex);
    CloseHandle(mutex);
    CloseHandle(saveFile);
    DestroyMenu(context.trayMenu);
    DestroyWindow(context.mainWindow);
    DeleteFile("traymond.dat"); // No save file means we have exited gracefully
    UnregisterHotKey(context.mainWindow, 0);
    return msg.wParam;
}
