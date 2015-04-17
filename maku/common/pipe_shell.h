#ifndef MAKU_COMMON_PIPE_SHELL_H_
#define MAKU_COMMON_PIPE_SHELL_H_

#include <ncore/ncore.h>
#include "pipe_msg_def.h"

namespace ncore
{
    class NamedPipe;
}
namespace maku
{

class PipeShell
{
    
public:
    enum Error
    {
        kErrorSuccess,
        kErrorEmpty,
        kErrorMsg,
        kErrorConnect,
    };

    typedef Error (PipeShell::*MsgHandle)();
public:
    PipeShell();

    virtual ~PipeShell();

protected:
    Error Pull();

    Error Push(const void * data, size_t len);
protected:
    virtual void OnHotKey();

    virtual void OnMouseEvent(PipeMouseEvent & e);

    virtual void OnKeyEvent(PipeKeyEvent & e);

    virtual void OnTextEvent(PipeTextEvent & e);

    virtual void OnCursorChangedEvent(PipeCursorChangedEvent & e);

    virtual void OnStatusChangedEvent(PipeStatusChangedEvent & e);

    virtual void OnPaintEvent(PipePaintEvent & e);
private:
    Error PullHotKey();

    Error PullMouseEvent();

    Error PullKeyEvent();

    Error PullTextEvent();

    Error PullCursorChangedEvent();

    Error PullStatusChangedEvent();

    Error PullPaintEvent();

    Error PullData(void * buffer, uint32_t size_to_read);
protected:
    ncore::NamedPipe * pipe_obj_;
private:
    MsgHandle msg_handle_[kMsgCount];


};

}
#endif
