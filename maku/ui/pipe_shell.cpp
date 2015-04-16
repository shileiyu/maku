#include "pipe_shell.h"

namespace maku
{

namespace ui
{

PipeShell::PipeShell()
{
    assert(0);
}

PipeShell::~PipeShell()
{
    ;
}

bool PipeShell::Connect(uint32_t flag)
{
    using namespace ncore;

    char pipename[MAX_PATH] = { 0 };
    sprintf_s(pipename, "%s%u", kPipeName, flag);
    pipe_obj_.reset(new NamedPipeClient);

    return pipe_obj_->init(pipename, PipeDirection::kDuplex, PipeOption::kNone, 0);
}

}

}