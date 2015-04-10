#ifndef MAKU_RENDER_CONTEXT_H_
#define MAKU_RENDER_CONTEXT_H_

#include <ncore\sys\spin_lock.h>
#include <ncore\sys\named_pipe.h>
#include <ncore\base\buffer.h>
#include "..\common\pipe_msg_def.h"
#include "canvas.h"

namespace maku
{
namespace render
{

class RenderContext
{
    typedef void (RenderContext::*MsgHandle)(void);
public:
    RenderContext(void);

    ~RenderContext(void);
public:
    bool IsPresent();            //是否呼出overlay

    void UpdateStatus(uint32_t width, uint32_t height);

    void Draw(Canvas * canvas);

    void HandleHookMsg(const MSG & msg);

    static RenderContext * Get();
private:
//管道名
    bool InitializePipe();

    void FiniPipe();
//由UI端处理消息
    bool PostPipeMsg(uint32_t msg_type, void * buf, uint32_t size); 

    bool RecvPipeMsg(); //暂不处理

    bool PeekPipeMsg(uint32_t & bytes_avail);

    void HandleMsgInTestMode(const MSG & msg);

    void OnStatusChangedEvent();

    void OnCursorChangedEvent();

    void OnPaintEvent();

    bool StartRenderUIProcess();

    void ExitRenderUIProcess();
private:
    bool IsSkip();

    void HandleMsg(const MSG & msg);

    void HandleRecvFail();

    void HandleKeyMsg(const MSG & msg);

    void HandleMouseMsg(const MSG & msg);
private:
    bool show_;
    bool shield_;
    bool need_restart_;//是否需要启动进程
    DWORD delay_count_;//延时启动进程

    uint32_t cur_frame_count_;
    ncore::Buffer image_buf_;
    ncore::FixedBuffer<sizeof(CursorChangedEvent)> cursor_buf_;
    Point cursor_pos_;

    uint32_t width_;
    uint32_t height_;
    HANDLE ui_handle_;
    DWORD device_flag_;
    ncore::SpinLock locker_;
    std::shared_ptr<ncore::NamedPipeServer> pipe_obj_;
    MsgHandle msg_handle_[kMsgCount];
};
    }
}
#endif
