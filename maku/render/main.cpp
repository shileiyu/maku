
#include <ncore/ncore.h>
#include "kernel32_hooker.h"
#include "user32_hooker.h"
#include "input_hooker.h"
#include "dinput_hooker.h"

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

}