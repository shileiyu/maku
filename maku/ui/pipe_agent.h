#ifndef MAKU_UI_PIPE_AGENT_H_
#define MAKU_UI_PIPE_AGENT_H_

#include <common/pipe_shell.h>

namespace ncore
{
    class NamedPipeClient;
}
namespace maku
{

namespace ui
{

class PipeAgent : public PipeShell
{
public:
    static PipeAgent * Get();

    PipeAgent();

    ~PipeAgent();

    bool init(unsigned int flag);

    void fini();

    bool Update();
protected:
    void OnHotKey() override;

    void OnMouseEvent(PipeMouseEvent & e) override;

    void OnKeyEvent(PipeKeyEvent & e) override;

    void OnTextEvent(PipeTextEvent & e) override;
private:
    ncore::NamedPipeClient * client_;
};

}



}

#endif