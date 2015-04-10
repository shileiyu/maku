#ifndef MAKU_RENDER_DINPUT_HOOKER_H_
#define MAKU_RENDER_DINPUT_HOOKER_H_

#include <ncore\sys\spin_lock.h>

namespace maku
{
namespace render
{
class DinputHooker
{
private:
    static ncore::SpinLock locker_;
public:
    static void Hook();

    static void ToggleBlocked(bool toggle);// unacquire mouse
};
}
}

#endif