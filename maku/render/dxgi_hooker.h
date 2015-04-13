#ifndef MAKU_RENDER_DXGI_HOOKER_H_
#define MAKU_RENDER_DXGI_HOOKER_H_

#include <ncore\sys\spin_lock.h>

namespace maku
{
namespace render
{
class DXGIHooker
{
private:
	static ncore::SpinLock locker_;
public:
    static void Hook();
};
}
}

#endif