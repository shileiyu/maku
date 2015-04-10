//without this macro that will lead to confilcit with winsock2 header
//we define it ourself.

#include "user32_hooker.h"
#include "detours\detours.h"
#include "detours\detours_ext.h"
#include "dinput_hooker.h"
#include "..\common\pipe_msg_def.h"
#include "utils.h"

namespace maku
{
namespace render
{
//user32 hook 
bool shield = false;
int game_cursor_count = 0;
POINT cursor_pos = {-1, -1};

//function type define
typedef int     (WINAPI *ShowCursor_T)(BOOL bShow);
typedef BOOL    (WINAPI *GetCursorPos_T)(LPPOINT lpPoint);
typedef BOOL    (WINAPI *SetCursorPos_T)(int X,int Y);
typedef HCURSOR (WINAPI *SetCursor_T)(HCURSOR hCursor);
typedef HWND    (WINAPI *SetCapture_T)(HWND hWnd);
typedef SHORT   (WINAPI *GetAsyncKeyState_T)(int vKey);
typedef int     (WINAPI *GetKeyNameTextA_T)(LONG lParam,LPTSTR lpString,int nSize);
typedef int     (WINAPI *GetKeyNameTextW_T)(LONG lParam,LPTSTR lpString,int nSize);
typedef UINT    (WINAPI *GetRawInputData_T)(HRAWINPUT hRawInput,UINT uiCommand,LPVOID pData,PUINT pcbSize,UINT cbSizeHeader);
typedef BOOL    (WINAPI *ClipCursor_T)(const RECT *lpRect);
typedef BOOL    (WINAPI *GetClipCursor_T)(LPRECT lpRect);
typedef UINT    (WINAPI *GetRawInputBuffer_T)(PRAWINPUT pData,PUINT pcbSize,UINT cbSizeHeader);
typedef LRESULT (WINAPI *DispatchMessageA_T)(_In_  const MSG *lpmsg);
typedef LRESULT (WINAPI *DispatchMessageW_T)(_In_  const MSG *lpmsg);

typedef LRESULT (WINAPI *SendMessageA_T)(
    _In_  HWND hWnd,
    _In_  UINT Msg,
    _In_  WPARAM wParam,
    _In_  LPARAM lParam
    );

typedef LRESULT (WINAPI *SendMessageW_T)(
    _In_  HWND hWnd,
    _In_  UINT Msg,
    _In_  WPARAM wParam,
    _In_  LPARAM lParam
    );

typedef BOOL (WINAPI *PostMessageA_T)(
    _In_opt_  HWND hWnd,
    _In_      UINT Msg,
    _In_      WPARAM wParam,
    _In_      LPARAM lParam
    );
typedef BOOL (WINAPI *PostMessageW_T)(
    _In_opt_  HWND hWnd,
    _In_      UINT Msg,
    _In_      WPARAM wParam,
    _In_      LPARAM lParam
    );
typedef BOOL (WINAPI *GetKeyboardState_T)(PBYTE lpKeyState);

typedef SHORT (WINAPI *GetKeyState_T)(int nVirtKey);

//ptr to detoured function
ShowCursor_T TShowCursor = 0;
GetCursorPos_T TGetCursorPos = 0;
SetCursorPos_T TSetCursorPos = 0;
SetCursor_T TSetCursor = 0;
GetAsyncKeyState_T TGetAsyncKeyState = 0;
GetRawInputData_T TGetRawInputData = 0;
ClipCursor_T TClipCursor = 0;
GetClipCursor_T TGetClipCursor = 0;
GetRawInputBuffer_T TGetRawInputBuffer = 0;

DispatchMessageA_T TDispatchMessageA = 0;
DispatchMessageW_T TDispatchMessageW = 0;

SendMessageA_T TSendMessageA = 0;
SendMessageW_T TSendMessageW = 0;
PostMessageA_T TPostMessageA = 0;
PostMessageW_T TPostMessageW = 0;

GetKeyboardState_T TGetKeyboardState = 0;
GetKeyState_T TGetKeyState = 0;

//detour function predeclared
SHORT   WINAPI FGetAsyncKeyState(int vKey);
int     WINAPI FGetKeyNameTextA(LONG lParam,LPTSTR lpString,int nSize);
int     WINAPI FGetKeyNameTextW(LONG lParam,LPTSTR lpString,int nSize);
UINT    WINAPI FGetRawInputData(HRAWINPUT hRawInput,UINT uiCommand,LPVOID pData,PUINT pcbSize,UINT cbSizeHeader);
BOOL    WINAPI FGetCursorPos(LPPOINT lpPoint);
int     WINAPI FShowCursor(BOOL bShow);
BOOL    WINAPI FSetCursorPos(int X,int Y);
HWND    WINAPI FSetCapture(HWND hWnd);
BOOL    WINAPI FClipCursor(const RECT *lpRect);
BOOL    WINAPI FGetClipCursor(LPRECT lpRect);
UINT    WINAPI FGetRawInputBuffer(PRAWINPUT pData,PUINT pcbSize,UINT cbSizeHeader);
BOOL    WINAPI FGetKeyboardState(PBYTE lpKeyState);
SHORT   WINAPI FGetKeyState(int nVirtKey);
LRESULT WINAPI FDispatchMessageA(_In_  const MSG *lpmsg);
LRESULT WINAPI FDispatchMessageW(_In_  const MSG *lpmsg);
void SaveStatus();
void RestoreStatus();
//////////////////////////////////////////////////////////////////////////
BOOL WINAPI FGetKeyboardState(PBYTE lpKeyState)
{
    if(shield)
    {
        memset(lpKeyState, 0, 256);
        return TRUE;
    }
    else
        return TGetKeyboardState(lpKeyState);
}

SHORT WINAPI FGetKeyState(int nVirtKey)
{
    if(shield)
    {
        return 0;
    }
    else
        return TGetKeyState(nVirtKey);
}
SHORT WINAPI FGetAsyncKeyState(int vKey)
{
    //if (RenderContext::Get()->is_hotkey_called())
    //{//直接屏蔽 三位一体1和2在呼出状态时 alt+enter切换窗口焦点后 再切回去 窗口不处理消息
    //    return -1;
    //}
    if(shield)
    {
        return 0;
    }
    else
        return TGetAsyncKeyState(vKey);
}

UINT WINAPI FGetRawInputData(HRAWINPUT hRawInput,
    UINT uiCommand,LPVOID pData,PUINT pcbSize,UINT cbSizeHeader)
{
    UINT ret = TGetRawInputData(hRawInput,uiCommand,pData,pcbSize,cbSizeHeader);
    if(shield)
    {
        if(pcbSize)
            *pcbSize = 0;
    }
    return ret;
}

BOOL WINAPI FGetCursorPos(LPPOINT lpPoint)
{
    if(shield)
    {//屏蔽状态下返回 保存的cursor_pos
        if(lpPoint)
            *lpPoint = cursor_pos;

        return TRUE;
    }
    else
        return TGetCursorPos(lpPoint);
}

int WINAPI FShowCursor(BOOL bShow)
{
    if (shield)
    {//呼出后 游戏主动隐藏光标显示光标 (主要是放电影的时候 呼出)
        int game_count = game_cursor_count;
        game_count += (bShow ? 1 : -1); 
        game_cursor_count = game_count;
        return game_count;
    }
    else
        return TShowCursor(bShow);
}

BOOL WINAPI FSetCursorPos(int X,int Y)
{
    if (shield)
    {
        cursor_pos.x = X;
        cursor_pos.y = Y;
        return TRUE;
    }
    else
        return TSetCursorPos(X,Y);
}

UINT WINAPI FGetRawInputBuffer(PRAWINPUT pData,PUINT pcbSize,UINT cbSizeHeader)
{
    UINT ret = TGetRawInputBuffer(pData,pcbSize,cbSizeHeader);
    if (shield)
        ret = 0;
    return ret;
}

LRESULT WINAPI FSendMessageA(
    _In_  HWND hWnd,
    _In_  UINT Msg,
    _In_  WPARAM wParam,
    _In_  LPARAM lParam
    )
{
    if(shield)
    {
        if( (Msg >= WM_MOUSEFIRST && Msg <= WM_MOUSELAST) ||
            (Msg >= WM_KEYFIRST && Msg <= WM_KEYLAST) )
        {//只屏蔽  鼠标和键盘
            return 0;
        }
    }
    return TSendMessageA(hWnd, Msg, wParam, lParam);
}

LRESULT WINAPI FSendMessageW(
    _In_  HWND hWnd,
    _In_  UINT Msg,
    _In_  WPARAM wParam,
    _In_  LPARAM lParam
    )
{
    if(shield)
    {
        if( (Msg >= WM_MOUSEFIRST && Msg <= WM_MOUSELAST) ||
            (Msg >= WM_KEYFIRST && Msg <= WM_KEYLAST) )
        {//只屏蔽  鼠标和键盘
            return 0;
        }
    }
    return TSendMessageW(hWnd, Msg, wParam, lParam);
}


BOOL WINAPI FPostMessageA(
    _In_opt_  HWND hWnd,
    _In_      UINT Msg,
    _In_      WPARAM wParam,
    _In_      LPARAM lParam
    )
{
    if(shield)
    {
        if( (Msg >= WM_MOUSEFIRST && Msg <= WM_MOUSELAST) ||
            (Msg >= WM_KEYFIRST && Msg <= WM_KEYLAST) )
        {//只屏蔽  鼠标和键盘
            return true;
        }
    }
    return TPostMessageA(hWnd, Msg, wParam, lParam);
}

BOOL WINAPI FPostMessageW(
    _In_opt_  HWND hWnd,
    _In_      UINT Msg,
    _In_      WPARAM wParam,
    _In_      LPARAM lParam
    )
{
    if(shield)
    {
        if( (Msg >= WM_MOUSEFIRST && Msg <= WM_MOUSELAST) ||
            (Msg >= WM_KEYFIRST && Msg <= WM_KEYLAST) )
        {//只屏蔽  鼠标和键盘            
            return true;
        }
    }
    return TPostMessageW(hWnd, Msg, wParam, lParam);
}
//////////////////////////////////////////////////////////////////////////
ncore::SpinLock User32Hooker::locker_;

#define SAVE_T_ADDR(m,f) T##f=(f##_T)GetProcAddress(m, #f);
#define DETOUR_ATTACH(f) if(T##f) { DetourAttach(&(PVOID&)T##f, F##f); }

void User32Hooker::Hook()
{
    void * module = ::GetModuleHandle(L"user32.dll");
    if(module == 0)
        return;

    //see also: kernel_hooker.cpp
    if(!IsModuleDetoured(module))
    {
        locker_.Acquire();
        if(!IsModuleDetoured(module))
        {
            FARPROC proc = 0;
            HMODULE mod = (HMODULE)module;

            SAVE_T_ADDR(mod,ShowCursor);
            SAVE_T_ADDR(mod,GetCursorPos);
            SAVE_T_ADDR(mod,SetCursorPos);
            SAVE_T_ADDR(mod,SetCursor);
            SAVE_T_ADDR(mod,GetAsyncKeyState);
            SAVE_T_ADDR(mod,GetKeyState);
            SAVE_T_ADDR(mod, GetKeyboardState);
            SAVE_T_ADDR(mod,GetRawInputData);
            SAVE_T_ADDR(mod,ClipCursor);
            SAVE_T_ADDR(mod,GetClipCursor);
            SAVE_T_ADDR(mod,GetRawInputBuffer);

            SAVE_T_ADDR(mod,SendMessageA);
            SAVE_T_ADDR(mod,SendMessageW);
            SAVE_T_ADDR(mod,PostMessageA);
            SAVE_T_ADDR(mod,PostMessageW);

            HANDLE curr_thread = GetCurrentThread();
            DetourTransactionBegin();
            DetourUpdateThread(curr_thread);

            DETOUR_ATTACH(ShowCursor);
            DETOUR_ATTACH(GetCursorPos);
            DETOUR_ATTACH(SetCursorPos);
            DETOUR_ATTACH(GetAsyncKeyState);
            DETOUR_ATTACH(GetKeyState);
            DETOUR_ATTACH(GetKeyboardState);
            DETOUR_ATTACH(GetRawInputData);
            DETOUR_ATTACH(GetRawInputBuffer);

            DETOUR_ATTACH(SendMessageA);
            DETOUR_ATTACH(SendMessageW);
            DETOUR_ATTACH(PostMessageA);
            DETOUR_ATTACH(PostMessageW);

            //if every thing is ok. mark this module detoured.
            if(!DetourTransactionCommit())
                MarkModuleDetoured(module);
        }
        locker_.Release();
    }
}

SHORT WINAPI User32Hooker::OrgGetAsyncKeyState(int vKey)
{
    return TGetAsyncKeyState(vKey);
}

BOOL WINAPI User32Hooker::OrgGetCursorPos(LPPOINT lpPoint)
{
    return TGetCursorPos(lpPoint);
}

BOOL WINAPI User32Hooker::OrgSetCursorPos(int x, int y)
{
    return TSetCursorPos(x, y);
}

BOOL WINAPI User32Hooker::OrgClipCursor(CONST RECT * lpRect)
{
    return TClipCursor(lpRect);
}

void User32Hooker::StartShield()
{
    shield = true;
    SaveStatus();
}

void User32Hooker::EndShield()
{
    shield = false;
    RestoreStatus();
}

void SaveStatus()
{
    DinputHooker::ToggleBlocked(true);
    //保存光标位置
    TGetCursorPos(&cursor_pos);
    //保存光标计数
    game_cursor_count = TShowCursor(FALSE);
    game_cursor_count++;//得到正确的光标计数
    TShowCursor(TRUE);

    //隐藏系统光标
    while (TShowCursor(FALSE) >= 0);
    ////限制光标只能在窗口内移动
    //RECT window_rect;
    //::GetWindowRect(hwnd, &window_rect);
    //TClipCursor(&window_rect);
}

void RestoreStatus()
{
    //恢复光标计数
    int cur_cursor_count = TShowCursor(FALSE);
    cur_cursor_count++;
    TShowCursor(TRUE);
    //
    if(game_cursor_count != cur_cursor_count)
    {
        bool bshow = game_cursor_count > cur_cursor_count;
        int ret = 0;
        while( ( ret = TShowCursor(bshow) ) != 
            game_cursor_count);
    }

    //恢复光标位置
    if(cursor_pos.x != -1 || cursor_pos.y != -1)
    {
        TSetCursorPos(cursor_pos.x, cursor_pos.y);
    }
    cursor_pos.x = cursor_pos.y = -1;
    DinputHooker::ToggleBlocked(false);
}

}
}