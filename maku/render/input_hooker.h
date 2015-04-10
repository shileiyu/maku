#ifndef MAKU_RENDER_INPUTHOOKER_H_
#define MAKU_RENDER_INPUTHOOKER_H_

#include <ncore\sys\spin_lock.h>

#define WM_CUSTOM_MESSAGE WM_USER + 100
#define WPARAM_HOTKEY 101
#define WPARAM_SHIELD 102

namespace maku
{
namespace render
{

class InputHooker
{
private:
    static ncore::SpinLock locker_;
public:
    static void CheckHotKey();

    static HWND GetCurrentHWND();

    static void CallOut(bool b);

    static void SetWindowed(bool b);

    static void SetTransRect(RECT & g_rect, RECT & b_rect);

    static void Hook();

};
}
}
#endif