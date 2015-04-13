#ifndef MAKU_RENDER_D3D9_HOOKER_H_
#define MAKU_RENDER_D3D9_HOOKER_H_

#include <ncore\sys\spin_lock.h>

namespace maku
{
namespace render
{
class D3D9Hooker
{
private:
    static ncore::SpinLock locker_;
public:
    static void Hook();
};
}
}

#endif