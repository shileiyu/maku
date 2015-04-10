#include "detours_ext.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Psapi.h>
#undef WIN32_LEAN_AND_MEAN
#endif
#include <stdint.h>

bool InCurrentModule(void * address)
{
    static char * begin = 0;
    static char * end = 0;
    if(begin == 0 && end == 0)
    {
        DWORD flags = GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT | 
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS;
        HMODULE module = 0;
        GetModuleHandleEx(flags, (LPCWSTR)InCurrentModule, &module);

        MODULEINFO info = {0};
        GetModuleInformation(GetCurrentProcess(), module, &info, sizeof(info));
        begin = (char *)info.lpBaseOfDll;
        end = begin + info.SizeOfImage;
    }

    return address >=  begin && address < end;
}

bool IsModuleDetoured(void * module)
{
    auto dos_header = (IMAGE_DOS_HEADER *)module;
    return dos_header->e_oemid == 0xdeed;
}

void MarkModuleDetoured(void * module)
{
    const size_t dos_header_size = sizeof(_IMAGE_DOS_HEADER);
    DWORD exflag = 0;
    auto dos_header = (IMAGE_DOS_HEADER *)module;

    if(VirtualProtect(dos_header, dos_header_size, PAGE_READWRITE, &exflag))
    {
        dos_header->e_oemid = 0xdeed;
        VirtualProtect(dos_header, dos_header_size, exflag, &exflag);
    }
    return;
}

bool IsVFDetoured(void * i7e, size_t idx)
{
    void ** obj = (void **)i7e;
    if(obj == 0)
        return 0;

    void ** vft = (void **)*obj;
    if(vft == 0)
        return 0;

    unsigned char * vfa = (unsigned char *)vft[idx];
    void * offset = *(void **)(vfa + 1);

    if( (vfa[0] == 0xE9) )
    {
        void * dst = vfa + (int32_t)offset + 5;
        return InCurrentModule(dst);
    }

    return false;
}
