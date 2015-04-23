#ifndef MAKU_UI_BACKROOM_H_
#define MAKU_UI_BACKROOM_H_

#include "plugin.h"
#include <common/pipe_shell.h>
#include <ncore/sys/message_loop.h>


namespace ncore
{
class NamedPipeClient;
class Buffer;
}

namespace maku
{
namespace ui
{

enum ErrorCode
{
    kErrorSuccess = 0,
    kGeneralFailure = -1,
    kInvalidParams = -2,
    kErrorPipe = -3,
    kErrorPlugin = -4,
};

class Backroom : public ncore::MessageLoop::Observer, public Controller, public PipeShell
{
    typedef void(__cdecl *CF)(Controller &);
    struct ModuleInfo
    {
        HMODULE module;
        CF load;
        CF unload;
    };
public:
    static Backroom * Get();
    
    Backroom();

    ~Backroom();

    ErrorCode Run();

protected:
    uint32_t OnIdle(ncore::MessageLoop & loop) override;
private:
    bool Update();

    void LoadPlugin();

    void UnloadPlugin();

    void Display(bool show, bool shield) override;

    void Redraw(const RedrawEvent & e) override;

    size_t GetWidth() override;

    size_t GetHeight() override;

    void OnHotKey() override;

    void OnMouseEvent(PipeMouseEvent & e) override;

    void OnKeyEvent(PipeKeyEvent & e) override;

    //void OnTextEvent(PipeTextEvent & e) override;
private:
    void AdjustCache(size_t size);
private:
    ncore::NamedPipeClient * client_;
    std::vector<ModuleInfo> infos_;
    std::vector<Plugin> plugins_;
    size_t width_;
    size_t height_;
    ncore::Buffer * cache_;
};

}
}

#endif