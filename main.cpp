#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_set>

static std::unordered_set<HWND> handled_windows;

std::string getWindowName(HWND hwnd)
{
    int length = GetWindowTextLength(hwnd);

    std::string buffer(length, '\0');
    if (length > 0)
    {
        GetWindowText(hwnd, buffer.data(), length + 1);
        return buffer;
    }

    return buffer;
}

bool FilterWindow(HWND hwnd)
{
    std::string buffer = getWindowName(hwnd);
    // if (buffer.empty())
    //     return false;
    // std::cout << buffer << std::endl;
    static const char *blacklist[] = {
        "Task Switching",
        "Microsoft Text Input Application",
        "Settings"};

    for (auto c : blacklist)
        if (buffer == c)
            return false;

    // if (handled_windows.count(hwnd))
    //     return false;

    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);

    if (!(style & WS_VISIBLE))
        return false;

    // Skip tool windows, popups, etc.
    if (exStyle & WS_EX_TOOLWINDOW)
        return false;

    if (GetAncestor(hwnd, GA_ROOT) != hwnd)
        return false;

    return true;
}

void HandleWindowDiscovered(HWND hwnd)
{
    if (!FilterWindow(hwnd))
        return;

    // handled_windows.insert(hwnd);

    std::string buffer = getWindowName(hwnd);

    std::cout << "Discovered Window " << buffer << std::endl;
}

void HandleWindowDestroyed(HWND hwnd)
{
    // auto it = handled_windows.find(hwnd);

    // if (it == handled_windows.end())
    //     return;
    // std::string buffer = getWindowName(hwnd);

    // std::cout << "Handling window deletion " << buffer << std::endl;
    // handled_windows.erase(hwnd);
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{

    HandleWindowDiscovered(hwnd);

    return TRUE;
}

// Callback function
void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd,
                           LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
    if (hwnd != NULL && idObject == OBJID_WINDOW)
    {
        if (event == EVENT_OBJECT_DESTROY)
        {
            HandleWindowDestroyed(hwnd);
        }
        else if (event == EVENT_OBJECT_SHOW)
        {
            HandleWindowDiscovered(hwnd);
        }
    }
}

int main()
{
    // Hook all processes/threads
    HWINEVENTHOOK hHook = SetWinEventHook(
        EVENT_OBJECT_DESTROY, EVENT_OBJECT_SHOW, // Or EVENT_SYSTEM_FOREGROUND
        NULL, WinEventProc, 0, 0,
        WINEVENT_OUTOFCONTEXT);
    EnumWindows(EnumWindowsProc, 0);
    if (hHook)
    {
        MSG msg;
        // Message loop required to receive events
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        UnhookWinEvent(hHook);
    }
    return 0;
}
