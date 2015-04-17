#include "backroom.h"
#include "pipe_agent.h"
#include <ncore/utils/karma.h>
#include <ncore/sys/options_parser.h>
#include <nui/base/pixpainter.h>

namespace maku
{
namespace ui
{

int ParseInteger(const std::string & litera, int _default, int min, int max)
{
    if (litera.length() == 0)
        return _default;
    int value = atoi(litera.data());
    if (value < min || value > max)
        return _default;
    return value;
}

int ParseInteger(const std::string & litera, int _default)
{
    if (litera.length() == 0)
        return _default;
    return atoi(litera.data());
}

Backroom * Backroom::Get()
{
    static Backroom * room = nullptr;
    if (!room)
        room = new Backroom;
    return room;
}

Backroom::Backroom()
{
    ;
}

Backroom::~Backroom()
{
    ;
}

ErrorCode Backroom::Run()
{
    using namespace ncore;

    auto error_code = kErrorSuccess;
    
    auto cmdline = Karma::ToUTF8(::GetCommandLine());
    OptionsParser options_parser(cmdline);
    auto options = options_parser.GetOptions();
    int width = ParseInteger(options["width"], 800);
    int height = ParseInteger(options["height"], 600);
    int flag = ParseInteger(options["flag"], 0);

    //nui initialize
    //SkGraphics::Init()
    //初始化自身
    error_code = kGeneralFailure;
    if (InitView(width, height))
    {
        //初始化管道通信
        error_code = kErrorPipe;
        if (PipeAgent::Get()->init(flag))
        {//管道初始化成功
            using namespace ncore;
            MessageLoop::Current()->AddObserver(this);
            MessageLoop::Current()->Run();
            MessageLoop::Current()->RemoveObserver(this);
            PipeAgent::Get()->fini();
        }
        FiniView();
    }
    //nui uninitialize
    //SkGraphics::Term();
    return error_code;
}

uint32_t Backroom::OnIdle(ncore::MessageLoop & loop)
{
    using namespace ncore;
    //接收管道消息
    if (!PipeAgent::Get()->Update())
        MessageLoop::Current()->Exit(kErrorPipe);

    PushPixmap();
    return 30;
}

void Backroom::PenddingRedraw(nui::ScopedWorld world, const nui::Rect & rect)
{
    inval_rect_.Union(rect);
}

void Backroom::SetCursor(nui::ScopedWorld world, nui::CursorStyles cursor)
{
    ;
}

bool Backroom::InitView(int width, int height)
{
    using namespace nui;
    using namespace ncore;
    world_ = new GadgetWorld(this);
    back_buffer_ = Pixmap::Alloc(Size::Make(width, height));
    raw_buffer_ = new Buffer();
    //初始化其他View
    assert(0);
    //layout
    return world_ != nullptr && back_buffer_.IsEmpty();
}

void Backroom::FiniView()
{
    using namespace nui;
    delete raw_buffer_;
    back_buffer_ = Pixmap();
    world_.Reset();
}

void Backroom::PushPixmap()
{
    using namespace nui;
    if (inval_rect_.isEmpty())
        return;
    PixPainter painter(back_buffer_);
    world_->Draw(painter, inval_rect_);
    
    inval_rect_.SetEmpty();
}

}
}