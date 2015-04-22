
#include <ncore/utils/karma.h>
#include "kernel32_hooker.h"
#include "user32_hooker.h"
#include "input_hooker.h"
#include "dinput_hooker.h"
#include "dxgi_hooker.h"
#include "d3d9_hooker.h"
#include "ddraw_hooker.h"
#include "opengl_hooker.h"
#include "render_context.h"

void HookUp();

BOOL CALLBACK DllMain(HMODULE module, DWORD reason, LPVOID reverse)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        //We don't care thread-notifications.
        DisableThreadLibraryCalls(module);
        //install all hooks
        HookUp();
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        FreeLibraryAndExitThread(module, 0);
    }
    return true;
}


void HookUp()
{
    using namespace maku::render;
    Kernel32Hooker::Hook();
    User32Hooker::Hook();
    InputHooker::Hook();
    DXGIHooker::Hook();
    D3D9Hooker::Hook();
    DDrawHooker::Hook();
    DinputHooker::Hook();
    OpenGLHooker::Hook();
}

extern "C" __declspec(dllexport) void __cdecl SetWorkDirectory(const wchar_t * work_dir)
{
    using namespace maku::render;
    
    RenderContext::Get()->SetWorkDirectory(ncore::Karma::ToUTF8(work_dir).data());
}