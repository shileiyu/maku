#ifndef MAKU_UI_PIPE_SHELL_H_
#define MAKU_UI_PIPE_SHELL_H_

#include <ncore/ncore.h>
#include <ncore/sys/named_pipe.h>

#include <common/pipe_msg_def.h>

namespace maku
{
namespace ui
{

class PipeShell
{
    typedef void (PipeShell::*MsgHandle)();
public:
    PipeShell();

    ~PipeShell();

    bool Connect(uint32_t flag);

    bool Update();
private:
    std::shared_ptr<ncore::NamedPipeClient> pipe_obj_;
    MsgHandle msg_handle_[kMsgCount];

};
}

}
#endif
