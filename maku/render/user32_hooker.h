#ifndef MAKU_RENDER_USER32_HOOKER_H_
#define MAKU_RENDER_USER32_HOOKER_H_

#include <ncore\sys\spin_lock.h>

namespace maku
{
namespace render
{


class User32Hooker
{
private:
    static ncore::SpinLock locker_;
public:
    static void StartShield();

    static void EndShield();

    static SHORT WINAPI OrgGetAsyncKeyState(int vKey);

    static BOOL WINAPI OrgGetCursorPos(LPPOINT lpPoint);

    static BOOL WINAPI OrgSetCursorPos(int x, int y);

    static BOOL WINAPI OrgClipCursor(CONST RECT * lpRect);
public:
    static void Hook();
};
}
}

#endif