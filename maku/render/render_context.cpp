
#include "render_context.h"
#include <ncore/utils/karma.h>
#include <ncore/sys/registry.h>
#include <ncore/sys/path.h>

#include "user32_hooker.h"
#include "input_hooker.h"

#include "../common/key_event.h"
#include "../common/mouse_event.h"

namespace maku
{
namespace render
{

const int kSize = 30;
const uint32_t max_skip_frame_count = 200;
const uint32_t kPipeReadBufSize = 2 * 1024 * 1024;
const uint32_t kPipeWriteBufSize = 16 * 1024 * 1024;

RenderContext * render_context2_ = NULL;

RenderContext::RenderContext(void)
    : image_buf_(15*1024*1024), cursor_pos_(0, 0)
{
    show_ = false;
    shield_ = false;
    need_restart_ = false;
    delay_count_ = 0;

    ui_handle_ = NULL;
    cur_frame_count_ = 0;
    width_ = 0;
    height_ = 0;
    memset(image_buf_.data(), 0, 15*1024*1024);
    memset(cursor_buf_.data(), 0, cursor_buf_.capacity());

    msg_handle_[kStatusChangedEvent] = &RenderContext::OnStatusChangedEvent;
    msg_handle_[kPaintEvent] = &RenderContext::OnPaintEvent;
    msg_handle_[kCursorChangedEvent] = &RenderContext::OnCursorChangedEvent;
}

RenderContext::~RenderContext(void)
{
    ;
}

void RenderContext::Draw(Canvas * canvas)
{
    char* buf = image_buf_.data();
    Bitmap bmp_info;
    bmp_info.bits = buf;
    bmp_info.width = width_;
    bmp_info.height = height_;
    bmp_info.pitch = width_*4;
    Rect dest(0, 0, canvas->GetWidth(), canvas->GetHeight());
    canvas->DrawBitmap(bmp_info, dest);

    if(shield_)
    {//绘制光标
        CursorChangedEvent * e = (CursorChangedEvent *)cursor_buf_.data();

        bmp_info.bits = cursor_buf_.data();
        bmp_info.width = 32;
        bmp_info.height = 32;
        bmp_info.pitch = bmp_info.width * 4;
        //根据光标位置以及热点进行绘制
        dest.SetXYWH(cursor_pos_.x() - e->xhotspot, 
                        cursor_pos_.y() - e->yhotspot, 
                        bmp_info.width, bmp_info.height);
        canvas->DrawBitmap(bmp_info, dest);
    }
}

void RenderContext::HandleHookMsg(const MSG & msg)
{
    HandleMsg(msg);
}
//post msg to back client
void RenderContext::HandleMsg(const MSG & msg)
{
    if (msg.message == WM_CUSTOM_MESSAGE &&
        msg.wParam == WPARAM_HOTKEY)
        PostPipeMsg(kHotKey, NULL, 0);
    else if (msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST)
        HandleKeyMsg(msg);
    else if(msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST)
        HandleMouseMsg(msg);
    else if (msg.message >= WM_NCMOUSEMOVE && msg.message <= WM_NCXBUTTONDBLCLK)
        HandleMouseMsg(msg);
}

void RenderContext::HandleKeyMsg(const MSG & msg)
{
    PipeTextEvent pipe_text;
    ZeroMemory(&pipe_text, sizeof(pipe_text));
    PipeKeyEvent pipe_key;
    ZeroMemory(&pipe_key, sizeof(pipe_key));
    const uint16_t kMSB = 0x8000;

    uint32_t value = static_cast<uint32_t>(msg.wParam);
    if( (User32Hooker::OrgGetAsyncKeyState(VK_LSHIFT) & kMSB) ||
        (User32Hooker::OrgGetAsyncKeyState(VK_RSHIFT) & kMSB) )
    {
        value |= KeyEvent::kShift;
    }

    if( (User32Hooker::OrgGetAsyncKeyState(VK_LCONTROL) & kMSB) ||
        (User32Hooker::OrgGetAsyncKeyState(VK_RCONTROL) & kMSB) )
    {
        value |= KeyEvent::kControl;
    }

    if( (User32Hooker::OrgGetAsyncKeyState(VK_LMENU) & kMSB) ||
        (User32Hooker::OrgGetAsyncKeyState(VK_RMENU) & kMSB) )
    {
        value |= KeyEvent::kAlt;
    }

    KeyEvent::KeyValue key_value = static_cast<KeyEvent::KeyValue>(value);
    pipe_key.value = key_value;

    switch (msg.message)
    {
    case WM_KEYDOWN:
        pipe_key.type = KeyEvent::kKeyDown;
        PostPipeMsg(kKeyEvent, &pipe_key, sizeof(pipe_key));
        break;
    case WM_KEYUP:
        pipe_key.type = KeyEvent::kKeyUp;
        PostPipeMsg(kKeyEvent, &pipe_key, sizeof(pipe_key));
        break;
    case WM_CHAR:
        pipe_text.text = (wchar_t)msg.wParam;
        PostPipeMsg(kTextEvent, &pipe_text, sizeof(pipe_text));
        break;
    }
}

void RenderContext::HandleMouseMsg(const MSG & msg)
{
    PipeMouseEvent pipe_mouse;
    ZeroMemory(&pipe_mouse, sizeof(pipe_mouse));
    pipe_mouse.x = static_cast<int16_t>(msg.lParam);
    pipe_mouse.y = static_cast<int16_t>(msg.lParam >> 16);
    uint32_t button = 0;
    if(msg.wParam & MK_LBUTTON)
        button |= MouseEvent::kLeft;
    if(msg.wParam & MK_RBUTTON)
        button |= MouseEvent::kRight;
    if(msg.wParam & MK_MBUTTON)
        button |= MouseEvent::kMiddle;
    if(msg.wParam & MK_XBUTTON1)
        button |= MouseEvent::kXButton1;
    if(msg.wParam & MK_XBUTTON2)
        button |= MouseEvent::kXButton2;
    pipe_mouse.state = button;

    switch (msg.message)
    {
    case WM_NCMOUSEMOVE:
    case WM_MOUSEMOVE:
            pipe_mouse.type = MouseEvent::kMouseMove;
        break;
    case WM_NCLBUTTONDBLCLK:
    case WM_LBUTTONDBLCLK:
            pipe_mouse.button = MouseEvent::kLeft;
            pipe_mouse.type = MouseEvent::kMouseDoubleClick;
        break;
    case WM_NCLBUTTONDOWN:
    case WM_LBUTTONDOWN:
            pipe_mouse.button = MouseEvent::kLeft;
            pipe_mouse.type = MouseEvent::kMouseDown;
        break;
    case WM_NCLBUTTONUP:
    case WM_LBUTTONUP:
            pipe_mouse.button = MouseEvent::kLeft;
            pipe_mouse.type = MouseEvent::kMouseUp;
        break;
    case WM_NCRBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
            pipe_mouse.button = MouseEvent::kRight;
            pipe_mouse.type = MouseEvent::kMouseDoubleClick;
        break;
    case WM_NCRBUTTONDOWN:
    case WM_RBUTTONDOWN:
            pipe_mouse.button = MouseEvent::kRight;
            pipe_mouse.type = MouseEvent::kMouseDown;
        break;
    case WM_NCRBUTTONUP:
    case WM_RBUTTONUP:
            pipe_mouse.button = MouseEvent::kRight;
            pipe_mouse.type = MouseEvent::kMouseUp;
        break;
    case WM_MOUSELEAVE:
            pipe_mouse.type = MouseEvent::kMouseLeave;
        break;
    case WM_MOUSEWHEEL:
            pipe_mouse.dy = HIWORD(msg.wParam);
            pipe_mouse.type = MouseEvent::kMouseWheel;
        break;
    case WM_MOUSEHWHEEL:
            pipe_mouse.dx = HIWORD(msg.wParam);
            pipe_mouse.type = MouseEvent::kMouseWheel;
        break;
    }
    //记录鼠标光标位置
    cursor_pos_.set_x(pipe_mouse.x);
    cursor_pos_.set_y(pipe_mouse.y);
    PostPipeMsg(kMouseEvent, &pipe_mouse, sizeof(pipe_mouse));
}

RenderContext * RenderContext::Get()
{
    if(render_context2_ == NULL)
    {
        render_context2_ = new RenderContext();
    }
    return render_context2_;
}

void RenderContext::ExitRenderUIProcess()
{
    memset(image_buf_.data(), 0, image_buf_.capacity());
    if(ui_handle_)
    {
        ::CloseHandle(ui_handle_);
        ui_handle_ = 0;
    }
}

bool RenderContext::IsSkip()
{//是否需要跳帧
    bool bret = cur_frame_count_ < max_skip_frame_count;
    if(bret)
        cur_frame_count_++;
    return  bret;
}

bool RenderContext::StartRenderUIProcess()
{
    using namespace ncore;

    ExitRenderUIProcess();
    FiniPipe();
    memset(image_buf_.data(), 0, image_buf_.capacity());

    if( !InitializePipe() )//初始化管道
        return false;

    const char * parts[] = {work_dir_.data(), "maku_ui.exe"};
    auto ui_path8 = Path::JoinPath(parts);
    auto ui_path = Karma::FromUTF8(ui_path8);					

    auto value_path = Karma::FromUTF8(work_dir_);
    PROCESS_INFORMATION processInfo;
    STARTUPINFO startUpInfo;
    memset(&processInfo, 0, sizeof(PROCESS_INFORMATION));
    memset(&startUpInfo, 0, sizeof(STARTUPINFO));
    startUpInfo.cb = sizeof(STARTUPINFO);
    startUpInfo.dwFlags = STARTF_USESHOWWINDOW;
    startUpInfo.wShowWindow = SW_HIDE;

    auto cmd_line8 = Karma::Format("-id=%u -width=%d -height=%d", 
                                   device_flag_, width_, height_);
    auto cmd_line16 = Karma::FromUTF8(cmd_line8);
    wchar_t cmd_line[256] = {0};
    memcpy_s(cmd_line, 256 * 2, cmd_line16.data(), cmd_line16.size() * 2);

    //开始创建进程
    if (!CreateProcess(ui_path.data(), cmd_line, NULL, NULL, FALSE, NULL, NULL, 
        value_path.data(), &startUpInfo, &processInfo))
        return false;

    //等待进程初始化
    ::WaitForInputIdle(processInfo.hProcess, INFINITE);
    ::CloseHandle(processInfo.hThread);
    ui_handle_ = processInfo.hProcess;

    return true;
}

bool RenderContext::InitializePipe()
{
    pipe_obj_.reset(new ncore::NamedPipeServer);

    device_flag_ = ::GetTickCount();
    bool bret = false;
    char name[MAX_PATH];
    sprintf_s(name, "%s%u", kPipeName, device_flag_);

	return pipe_obj_->init(name, ncore::PipeDirection::kDuplex,
		ncore::PipeOption::kNone,
		ncore::PipeTransmissionMode::kStream, 2,
        kPipeWriteBufSize, kPipeReadBufSize, 0);
}

void RenderContext::FiniPipe()
{
    pipe_obj_.reset();
}

bool RenderContext::PostPipeMsg(uint32_t msg_type, void * buf, uint32_t size)
{
    if (pipe_obj_ == 0)
        return false;

    locker_.Acquire();
    uint32_t total_size = sizeof(PipeMsgHeader) + size;
	ncore::Buffer pipe_buff(total_size);
    PipeMsgHeader * msg_header;
    msg_header = reinterpret_cast<PipeMsgHeader*>(pipe_buff.data());
    msg_header->type = msg_type;
    if (size != 0)
        memcpy_s(msg_header + 1, size, buf, size);

    uint32_t trans_data = 0;
    bool ret = pipe_obj_->Write((void*)pipe_buff.data(), 
        pipe_buff.capacity(), trans_data);
    locker_.Release();
    return ret;
}

bool RenderContext::PeekPipeMsg(uint32_t & bytes_avail)
{
    if (pipe_obj_)
    {
        uint32_t bytes_msg = 0;
        uint32_t byte_read = 0;
        char buffer[100] = {0};
        if (pipe_obj_->Peek(buffer, 0, byte_read, bytes_avail, bytes_msg))
            return true;
    }
    return false;
}

bool RenderContext::RecvPipeMsg()
{
    if (pipe_obj_)
    {
        uint32_t bytes_avail = 0;
        if (PeekPipeMsg(bytes_avail))
        {
            if (bytes_avail >= sizeof(PipeMsgHeader))
            {
                uint32_t transfer = 0;
                uint32_t size = sizeof(PipeMsgHeader);
				ncore::Buffer recv_buf(size);
                pipe_obj_->Read(recv_buf.data(), size, transfer);

                const PipeMsgHeader * msg_header = 
                    reinterpret_cast<PipeMsgHeader*>(recv_buf.data());

                MsgHandle msg_handle = msg_handle_[msg_header->type];
                if (msg_handle != NULL)
                {
                    (this->*msg_handle)();
                }
                return true;
            }
        }//peek
        else
        {
            HandleRecvFail();
        }
    }
    return false;
}

void RenderContext::HandleRecvFail()
{
    if(ui_handle_ != 0)
    {
        DWORD wait_ret = ::WaitForSingleObject(ui_handle_, 0);
        if(WAIT_OBJECT_0 == wait_ret)
        {
            ExitRenderUIProcess();
            FiniPipe();
            memset(image_buf_.data(), 0xff, image_buf_.capacity());
            //InputHooker::SetCallOut(false);
            //show_ = false;
            //shield_ = false;

            //width_ = height_ = 0;
            //cur_frame_count_ = 0;
            need_restart_ = true;
            delay_count_ = ::GetTickCount();
        }
    }
}

void RenderContext::OnStatusChangedEvent()
{
    uint32_t transfer = 0;
    uint32_t size = sizeof(StatusChangedEvent);
	ncore::Buffer recv_buf(size);
    pipe_obj_->Read(recv_buf.data(), size, transfer);
    const StatusChangedEvent * sc = 
        reinterpret_cast<StatusChangedEvent*>(recv_buf.data());

    show_ = sc->show;
    shield_ = sc->shield;

    InputHooker::CallOut(sc->shield);
}

void RenderContext::OnCursorChangedEvent()
{//保存鼠标信息
    uint32_t transfer = 0;
    if(!pipe_obj_->Read(cursor_buf_.data(), cursor_buf_.capacity(), transfer))
        return;

    if(transfer != cursor_buf_.capacity())
        return;
}

void RenderContext::OnPaintEvent()
{
    uint32_t transfer = 0;
	ncore::FixedBuffer<sizeof(PaintEvent)> recv_buf;
    pipe_obj_->Read(recv_buf.data(), recv_buf.capacity(), transfer);
    const PaintEvent * pe = 
        reinterpret_cast<PaintEvent*>(recv_buf.data());

	ncore::Buffer data_buffer(pe->width * 4);
    uint32_t dst_pitch = width_ * 4;

	Rect intersect_rect;
    intersect_rect.SetLTRB(0, 0, width_, height_);
    Rect src_rect;
    src_rect.SetXYWH(pe->left, pe->top,  pe->width, pe->height);
    intersect_rect.Intersect(src_rect);
    
    for(uint32_t i = 0; i < pe->height; ++i)
    {
        pipe_obj_->Read(data_buffer.data(), data_buffer.capacity(), transfer);
        //交集为空则跳过 仅读取管道中数据
        if(intersect_rect.isEmpty())
            continue;
        //当前读取的像素所在行
        int col = i + pe->top;
        //当前的像素行在 交集区域中 需要复制
        if(col >= intersect_rect.top() && col < intersect_rect.bottom())
        {
            char * dst = image_buf_.data() + dst_pitch *col;
            dst += intersect_rect.left() * 4;
            //定位到需要复制的像素起始内存
            char * src = data_buffer.data();
            src += (intersect_rect.left() - src_rect.left()) * 4;
            memcpy(dst, src, intersect_rect.width() * 4);
        }
    } 
}

void RenderContext::UpdateStatus(uint32_t width, uint32_t height)
{
    if(IsSkip())
        return ;

    if(width_ != width || height_ != height)
    {
        ExitRenderUIProcess();
        FiniPipe();
        //不再恢复状态
        //InputHooker::SetCallOut(false);
        //show_ = false;
        //shield_ = false;

        width_ = width;
        height_ = height;
        need_restart_ = true;
        delay_count_ = ::GetTickCount();
    }

    if(need_restart_ && ::GetTickCount() - delay_count_ >= 2000)
    {
        need_restart_ = false;
        StartRenderUIProcess();
    }
    while(RecvPipeMsg());//处理管道消息          
}

bool RenderContext::IsPresent()
{
    return show_;
}

void RenderContext::SetWorkDirectory(const char * work_dir)
{
    if (work_dir)
        work_dir_ = work_dir;
}




}
}
