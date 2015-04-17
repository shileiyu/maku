#include "pipe_agent.h"
#include <ncore/sys/named_pipe.h>

namespace maku
{

namespace ui
{

PipeAgent * PipeAgent::Get()
{
    static PipeAgent * agent = nullptr;
    if (!agent)
        agent = new PipeAgent();

    return agent;
}

PipeAgent::PipeAgent()
:client_(nullptr)
{
    ;
}

PipeAgent::~PipeAgent()
{
    fini();
}



bool PipeAgent::init(unsigned int flag)
{
    using namespace ncore;
    char pipename[MAX_PATH] = { 0 };
    sprintf_s(pipename, "%s%u", kPipeName, flag);
    pipe_obj_ = client_ = new NamedPipeClient;
    return client_->init(pipename, PipeDirection::kDuplex, PipeOption::kNone, 0);
}

void PipeAgent::fini()
{
    if (client_)
    {
        delete client_;
        pipe_obj_ = client_ = nullptr;
    }
}

bool PipeAgent::Update()
{
    Error e = kErrorSuccess;

    while (kErrorSuccess == (e = Pull()) );

    return e == kErrorEmpty;
}

void PipeAgent::OnHotKey()
{
    ;
}

void PipeAgent::OnMouseEvent(PipeMouseEvent & e)
{

}

void PipeAgent::OnKeyEvent(PipeKeyEvent & e)
{

}

void PipeAgent::OnTextEvent(PipeTextEvent & e)
{
    ;
}

}

}