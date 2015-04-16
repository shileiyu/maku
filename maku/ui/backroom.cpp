#include "backroom.h"
#include <ncore/utils/karma.h>
#include <ncore/sys/options_parser.h>

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
:show_(false)
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
    show_ = options.HasOption("show");
    int width = ParseInteger(options["width"], 800);
    int height = ParseInteger(options["height"], 600);

    //nui initialize
    //SkGraphics::Init()

    if (show_)
    {//创建窗口 并显示
        ;
    }
    //初始化管道通信
    


    //消息循环


    //消息循环结束
    //nui uninitialize
    //SkGraphics::Term();


    return error_code;
}

}
}