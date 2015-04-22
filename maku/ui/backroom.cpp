#include "backroom.h"
#include <ncore/utils/karma.h>
#include <ncore/sys/options_parser.h>
#include <ncore/sys/named_pipe.h>
#include <ncore/base/buffer.h>

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
:client_(0), width_(0), height_(0), cache_(0)
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
    auto cmdline = Karma::ToUTF8(::GetCommandLine());
    OptionsParser options_parser(cmdline);
    auto options = options_parser.GetOptions();
    width_ = ParseInteger(options["width"], 800);
    height_ = ParseInteger(options["height"], 600);
    int flag = ParseInteger(options["flag"], 0);

    //nui initialize
    //SkGraphics::Init()
    //初始化管道通信
    char pipename[MAX_PATH] = { 0 };
    sprintf_s(pipename, "%s%u", kPipeName, flag);
    pipe_obj_ = client_ = new NamedPipeClient;
    if (!client_->init(pipename, PipeDirection::kDuplex, PipeOption::kNone, 0))
        return kErrorPipe;
    LoadPlugin();

    MessageLoop::Current()->AddObserver(this);
    MessageLoop::Current()->Run();
    MessageLoop::Current()->RemoveObserver(this);

    UnloadPlugin();
    client_->fini();
    delete client_;
    pipe_obj_ = client_ = nullptr;
    //nui uninitialize
    //SkGraphics::Term();
    return ErrorCode::kErrorSuccess;
}

bool Backroom::Update()
{
    Error e = kErrorSuccess;

    while (kErrorSuccess == (e = Pull()));

    return e == kErrorEmpty;
}

void Backroom::LoadPlugin()
{
    WIN32_FIND_DATA fd;
    HANDLE file = ::FindFirstFile(L"*.pv", &fd);
    if (INVALID_HANDLE_VALUE == file)
        return;
    do
    {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            ModuleInfo info;
            info.module = LoadLibrary(fd.cFileName);
            info.load = (CF)GetProcAddress(info.module, "Load");
            info.unload = (CF)GetProcAddress(info.module, "Unload");
            if (info.module && info.load && info.unload)
                infos_.push_back(info);
        }

    } while (::FindNextFile(file, &fd));

    for (auto iter = infos_.begin(); iter != infos_.end(); ++iter)
    {
        iter->load(*this);
    }
}

void Backroom::UnloadPlugin()
{
    for (auto iter = infos_.begin(); iter != infos_.end(); ++iter)
    {
        iter->unload(*this);
        FreeLibrary(iter->module);
    }
    infos_.clear();
}

void Backroom::Show(bool b)
{
    ;
}

void Backroom::Shield(bool b)
{
    ;
}

void Backroom::Redraw(const RedrawEvent & e)
{
    int size = e.subset_height * e.pitch +
        sizeof(PipePaintEvent) + sizeof(PipeMsgHeader);
    AdjustCache(size);
    PipeMsgHeader * head = reinterpret_cast<PipeMsgHeader*>(cache_->data());
    PipePaintEvent * body = reinterpret_cast<PipePaintEvent *>(head + 1);
    char * bits = reinterpret_cast<char *>(body + 1);

    head->type = kPaintEvent;
    body->left = e.subset_left;
    body->top = e.subset_top;
    body->width = e.subset_width;
    body->height = e.subset_height;
    //一行行的复制图像数据
    size_t body_pitch = body->width * 4;
    for (size_t i = 0; i < e.subset_height; ++i)
    {
        char * src = reinterpret_cast<char *>(e.bits) + 
            body->left + (body->top + i) * e.pitch;
        memcpy(bits + i * body_pitch, src, body_pitch);
    }
    Push(cache_->data(), size);
}

size_t Backroom::GetWidth()
{
    return width_;
}

size_t Backroom::GetHeight()
{
    return height_;
}


void Backroom::OnHotKey()
{
    for (auto iter = plugins_.begin(); iter != plugins_.end(); ++iter)
        (iter->OnHotKey)(*this);
}

void Backroom::OnMouseEvent(PipeMouseEvent & pe)
{
    MouseEvent e;
    memcpy(&e, &pe, sizeof(e));

    for (auto iter = plugins_.begin(); iter != plugins_.end(); ++iter)
        (iter->OnMouseEvent)(*this, e);
}

void Backroom::OnKeyEvent(PipeKeyEvent & pe)
{
    KeyEvent e;
    memcpy(&e, &pe, sizeof(e));
    for (auto iter = plugins_.begin(); iter != plugins_.end(); ++iter)
        (iter->OnKeyEvent)(*this, e);
}

uint32_t Backroom::OnIdle(ncore::MessageLoop & loop)
{
    using namespace ncore;
    //接收管道消息
    if (!Update())
        MessageLoop::Current()->Exit(kErrorPipe);
    for (auto iter = plugins_.begin(); iter != plugins_.end(); ++iter)
        (iter->OnIdle)(*this);
    return 30;
}

void Backroom::AdjustCache(size_t size)
{
    if (!cache_ || cache_->capacity() < size)
    {
        if (cache_)
            delete cache_;
        cache_ = new ncore::Buffer(size);
    }
    assert(cache_ && cache_->capacity() >= size);
}

}
}