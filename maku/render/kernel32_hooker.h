/*
* Hook Kernrel32.dll or KernelBase.dll
*/
#ifndef MAKU_RENDER_KERNEL32_HOOKER_H_
#define MAKU_RENDER_KERNEL32_HOOKER_H_

#include <ncore\sys\spin_lock.h>

namespace maku
{
namespace render
{
/*
Kernel Module (Kernel32.dll or KernelBase.dll) is loaded during progcess 
initialization and It won't be free until the process terminated.
So, we could assume it's always available.
*/
class Kernel32Hooker
{
private:
    static ncore::SpinLock locker_;
public:
    static void Hook();
};

}
}

#endif