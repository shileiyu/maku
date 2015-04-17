#include "pipe_shell.h"
#include <ncore/sys/named_pipe.h>
#include <ncore/base/buffer.h>

namespace maku
{

PipeShell::PipeShell()
:pipe_obj_(nullptr)
{
    
}

PipeShell::~PipeShell()
{
    memset(&msg_handle_, 0, sizeof(msg_handle_));

    msg_handle_[kHotKey] = &PipeShell::PullHotKey;
    msg_handle_[kMouseEvent] = &PipeShell::PullMouseEvent;
    msg_handle_[kKeyEvent] = &PipeShell::PullKeyEvent;
    msg_handle_[kTextEvent] = &PipeShell::PullTextEvent;
    msg_handle_[kCursorChangedEvent] = &PipeShell::PullCursorChangedEvent;
    msg_handle_[kStatusChangedEvent] = &PipeShell::PullStatusChangedEvent;
    msg_handle_[kPaintEvent] = &PipeShell::PullPaintEvent;
}

PipeShell::Error PipeShell::Pull()
{
    assert(pipe_obj_);

    if (!pipe_obj_)
        return kErrorConnect;

    ncore::FixedBuffer<sizeof(PipeMsgHeader)> buf;
    uint32_t bytes_avail = 0;
    uint32_t bytes_msg = 0;
    uint32_t read_bytes = 0;
    if (!pipe_obj_->Peek(0, 0, read_bytes, bytes_avail, bytes_msg))
        return kErrorConnect;

    //管道没有消息
    if (bytes_avail == 0)
        return kErrorEmpty;

    Error e;
    e = PullData(buf.data(), buf.capacity());
    if (kErrorSuccess == e)
    {//读取Header成功
        PipeMsgHeader * header = reinterpret_cast<PipeMsgHeader *>(buf.data());
        MsgHandle handle = msg_handle_[header->type];
        if (handle != 0)
            e = (this->*handle)();
    }
    return e;
}

PipeShell::Error PipeShell::Push(const void * data, size_t size)
{
    if (!pipe_obj_)
        return kErrorConnect;

    uint32_t wrote_bytes = 0;
    if (!pipe_obj_->Write(data, size, wrote_bytes) || wrote_bytes != size)
        return kErrorConnect;
    return kErrorSuccess;
}



PipeShell::Error PipeShell::PullHotKey()
{
    OnHotKey();
    return kErrorSuccess;
}

PipeShell::Error PipeShell::PullMouseEvent()
{
    ncore::FixedBuffer<sizeof(PipeMouseEvent)> buf;
    auto e = PullData(buf.data(), buf.capacity());
    if (e == kErrorSuccess)
    {
        auto msg = reinterpret_cast<PipeMouseEvent*>(buf.data());
        OnMouseEvent(*msg);
    }
    return e;
}

PipeShell::Error PipeShell::PullKeyEvent()
{
    ncore::FixedBuffer<sizeof(PipeKeyEvent)> buf;
    auto e = PullData(buf.data(), buf.capacity());
    if (e == kErrorSuccess)
    {
        auto msg = reinterpret_cast<PipeKeyEvent*>(buf.data());
        OnKeyEvent(*msg);
    }
    return e;
}

PipeShell::Error PipeShell::PullTextEvent()
{
    ncore::FixedBuffer<sizeof(PipeTextEvent)> buf;
    auto e = PullData(buf.data(), buf.capacity());
    if (e == kErrorSuccess)
    {
        auto msg = reinterpret_cast<PipeTextEvent*>(buf.data());
        OnTextEvent(*msg);
    }
    return e;
}

PipeShell::Error PipeShell::PullCursorChangedEvent()
{

    ncore::FixedBuffer<sizeof(PipeCursorChangedEvent)> buf;
    auto e = PullData(buf.data(), buf.capacity());
    if (e == kErrorSuccess)
    {
        auto msg = reinterpret_cast<PipeCursorChangedEvent*>(buf.data());
        OnCursorChangedEvent(*msg);
    }
    return e;
}

PipeShell::Error PipeShell::PullStatusChangedEvent()
{

    ncore::FixedBuffer<sizeof(PipeStatusChangedEvent)> buf;
    auto e = PullData(buf.data(), buf.capacity());
    if (e == kErrorSuccess)
    {
        auto msg = reinterpret_cast<PipeStatusChangedEvent*>(buf.data());
        OnStatusChangedEvent(*msg);
    }
    return e;
}

PipeShell::Error PipeShell::PullPaintEvent()
{
    ncore::FixedBuffer<sizeof(PipePaintEvent)> buf;
    auto e = PullData(buf.data(), buf.capacity());
    if (e == kErrorSuccess)
    {
        auto msg = reinterpret_cast<PipePaintEvent*>(buf.data());
        OnPaintEvent(*msg);
    }
    return e;
}


void PipeShell::OnHotKey()
{

}

void PipeShell::OnMouseEvent(PipeMouseEvent & e)
{
    ;
}

void PipeShell::OnKeyEvent(PipeKeyEvent & e)
{
    ;
}

void PipeShell::OnTextEvent(PipeTextEvent & e)
{
    ;
}

void PipeShell::OnCursorChangedEvent(PipeCursorChangedEvent & e)
{
    ;
}

void PipeShell::OnStatusChangedEvent(PipeStatusChangedEvent & e)
{
    ;
}

void PipeShell::OnPaintEvent(PipePaintEvent & e)
{
    ;
}

PipeShell::Error PipeShell::PullData(void * buffer, uint32_t size_to_read)
{
    uint32_t read_bytes = 0;
    if (pipe_obj_->Read(buffer, size_to_read, read_bytes))
    {
        if (read_bytes == size_to_read)
            return kErrorSuccess;
        else
            return kErrorMsg;
    }
    else
        return kErrorConnect;
}

}