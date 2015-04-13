#ifndef MAKU_RENDER_OPENGL_HOOKER_H_
#define MAKU_RENDER_OPENGL_HOOKER_H_

#include <ncore\sys\spin_lock.h>

namespace maku
{
namespace render
{
class OpenGLHooker
{
private:
    static ncore::SpinLock locker_;
public:
    static void Hook();
};
    }
}
#endif