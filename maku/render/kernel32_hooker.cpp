//without this macro that will lead to confilcit with winsock2 header
//we define it ourself.

#include "kernel32_hooker.h"
#include "detours\detours.h"
#include "detours\detours_ext.h"
#include "ddraw_hooker.h"
//#include "d3d8_hooker.h"
#include "d3d9_hooker.h"
#include "dxgi_hooker.h"
#include "dinput_hooker.h"
#include "opengl_hooker.h"

namespace maku
{
namespace render
{
//Define type of LoadLibraryExW
typedef HMODULE (WINAPI *LoadLibraryExW_t)(
    LPCWSTR lpLibFileName, 
    HANDLE hFile, 
    DWORD dwFlags);

//Pointer to True LoadLibraryExW.
//Caution! its value will be changed by Detour Functions.
LoadLibraryExW_t TLoadLibraryExW = 0;

//Our Detoured LoadLibraryExW.
//Just forward to original LoadLibraryExW then try to hook target module.
//target hooker decide whether do a hook procedure.
HMODULE WINAPI FLoadLibraryExW(
    LPCWSTR lpLibFileName, 
    HANDLE hFile, 
    DWORD dwFlags)
{
    HMODULE curr_mod = 0;
    if(TLoadLibraryExW)
    {
        curr_mod = TLoadLibraryExW(lpLibFileName, hFile, dwFlags);
        DXGIHooker::Hook();
        DinputHooker::Hook();
        D3D9Hooker::Hook();
        //D3D8Hooker::Hook();
        DDrawHooker::Hook();
        OpenGLHooker::Hook();
    }
    return curr_mod;
}

//speak by itself
ncore::SpinLock Kernel32Hooker::locker_;

void Kernel32Hooker::Hook()
{
    void * module = 0;
    OSVERSIONINFO osver = {0};

    osver.dwOSVersionInfoSize = sizeof(osver);
    GetVersionEx(&osver);
    DWORD major = osver.dwMajorVersion;
    DWORD minor = osver.dwMinorVersion;

    //only support xp, vist and win7.
    if(major != 5 && major != 6)
        return;

    //if it's running on win 7, hook KernelBase instead.
    if(major == 6  && minor == 1)
    {
        module = ::GetModuleHandle(L"kernelBase.dll");
    }
    else
    {
        module = ::GetModuleHandle(L"kernel32.dll");
    }

    //we can't find base address of kernel module. just ignore.
    //it seem like can't happen in real-world.
    if(module == 0)
        return;

    //make sure this module haven't detoured yet.
    //why we check it twice.
    //please searching double check lock pattern for further study.
    if(!IsModuleDetoured(module))
    {
        locker_.Acquire();
        if(!IsModuleDetoured(module))
        {
            //retrive address of LoadLibraryExW.
            HMODULE mod = (HMODULE)module;
            FARPROC proc = GetProcAddress(mod, "LoadLibraryExW");
            TLoadLibraryExW  = (LoadLibraryExW_t)proc;

            //hook it
            if(TLoadLibraryExW)
            {
                HANDLE curr_thread = GetCurrentThread();
                DetourTransactionBegin();
                DetourUpdateThread(curr_thread);
                DetourAttach(&(PVOID&)TLoadLibraryExW, FLoadLibraryExW);
                //if every thing is ok. mark kernel module detoured.
                if(!DetourTransactionCommit())
                    MarkModuleDetoured(module);
            }
        }
        locker_.Release();
    }
}

}
}