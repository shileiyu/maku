#include <ncore/ncore.h>
#include "demo.h"

using namespace maku::ui;

BOOL CALLBACK DllMain(HMODULE module, DWORD reason, LPVOID reverse)
{
    return true;
}

extern "C" __declspec(dllexport) void __cdecl Load(Controller & controller)
{
    PluginView::Get()->Load(controller);
}


extern "C" __declspec(dllexport) void __cdecl Unload(Controller & controller)
{
    PluginView::Get()->Unload(controller);
}