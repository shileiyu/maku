//dinput headers will refer to windows header.
//without this macro that will lead to conflict with winsock2 header
//we define it ourself.

#include "dinput_hooker.h"
#include "detours\detours.h"
#include "detours\detours_ext.h"

#include "dinput_common.h"
#include "utils.h"

namespace maku
{
namespace render
{
namespace dinputhooker
{

const GUID IID_IDirectInput8A_T = {0xBF798030,0x483A,0x4DA2,0xAA,
    0x99,0x5D,0x64,0xED,0x36,0x97,0x00};
const GUID IID_IDirectInput8W_T = {0xBF798031,0x483A,0x4DA2,0xAA,
    0x99,0x5D,0x64,0xED,0x36,0x97,0x00};
const GUID GUID_SysMouse_T =      {0x6F1D2B60,0xD5A0,0x11CF,0xBF,
    0xC7,0x44,0x45,0x53,0x54,0x00,0x00};
const GUID GUID_SysKeyboard =     {0x6F1D2B61,0xD5A0,0x11CF,0xBF,
    0xC7,0x44,0x45,0x53,0x54,0x00,0x00};

DirectInput8Create_T TDirectInput8Create = 0;
CreateDevice_T TCreateDeviceA = 0;
CreateDevice_T TCreateDeviceW = 0;
//LPDIRECTINPUTDEVICE8 Mouse和Keyboard设备 SetCooperativeLevel, Acquire, Release函数地址是一样的
SetCooperativeLevel_T TSetCooperativeLevel = 0;
Acquire_T TAcquire = 0;
Unacquire_T TUnacquire = NULL;
Release_T TRelease = 0;
GetDeviceState_T TGetDeviceState = 0;
GetDeviceData_T  TGetDeviceData = 0;

enum State
{
    kInit,
    kUnAcquire,
    kAcquire,
};

typedef LazyMap<LPDIRECTINPUTDEVICE8, State> Device2State;
//0->init     1->unacquire    2->acquire
Device2State device_state;

ncore::SpinLock locker;
bool blocked = false;
//////////////////////////////////////////////////////////////////////////
HRESULT WINAPI FGetDeviceState(LPDIRECTINPUTDEVICE8 self, DWORD length, LPVOID buf)
{
    if(blocked)
    {
        memset(buf, 0, length);
        return DI_OK;
    }
    else
        return TGetDeviceState(self, length, buf);
}

HRESULT(WINAPI FGetDeviceData)(LPDIRECTINPUTDEVICE8 self, 
    DWORD cbObjectData,LPDIDEVICEOBJECTDATA rgdod,
    LPDWORD pdwInOut,DWORD dwFlags)
{
    HRESULT hr;
    if(blocked)
    {
        locker.Acquire();
        if(device_state.Has(self))
        {
            *pdwInOut = 0;
            locker.Release();
            return DI_OK;
        }
        locker.Release();
    }

    hr = TGetDeviceData(self, cbObjectData, rgdod, pdwInOut, dwFlags);
    return hr;
}

HRESULT WINAPI FAcquire(LPDIRECTINPUTDEVICE8 self)
{
    locker.Acquire();
    if(device_state.Has(self))
    {
        device_state[self] = kAcquire;
        if (blocked)
        {
            locker.Release();
            return DI_OK;
        }
    }
    locker.Release();
    return TAcquire(self);
}

HRESULT WINAPI FUnacquire(LPDIRECTINPUTDEVICE8 self)
{
    locker.Acquire();
    if(device_state.Has(self))
    {
        device_state[self] = kUnAcquire;
        if (blocked)
        {
            locker.Release();
            return DI_OK;
        }
    }

    locker.Release();
    return TUnacquire(self);
}

HRESULT WINAPI FSetCooperativeLevel(LPDIRECTINPUTDEVICE8 self, 
    HWND hwnd, DWORD flag)
{
    return TSetCooperativeLevel(self, hwnd, flag);
}

HRESULT WINAPI FRelease(IUnknown * self)
{
    ULONG ret = TRelease(self);
    if(ret == 0)
    {//对象被释放
        locker.Acquire();

        if(device_state.Has((LPDIRECTINPUTDEVICE8)self))
        {
            device_state.Remove((LPDIRECTINPUTDEVICE8)self);
        }
        locker.Release();
    }
    return ret;
}

void DetourRelease(IUnknown * self)
{
    if(!IsVFDetoured(self, kIdxRelease))
    {
        if(GetVFAddress(self, kIdxRelease, TRelease))
            DetourAttach((void **)&TRelease, FRelease);
    }
}

void DetourAcquire(LPDIRECTINPUTDEVICE8 device)
{
    if(!IsVFDetoured(device, kIdxAcquire))
    {
        if(GetVFAddress(device, kIdxAcquire, TAcquire))
            DetourAttach((void **)&TAcquire, FAcquire);
    }
}

void DetourUAcquire(LPDIRECTINPUTDEVICE8 device)
{
    if(!IsVFDetoured(device, kIdxUnacquire))
    {
        if(GetVFAddress(device, kIdxUnacquire, TUnacquire))
            DetourAttach((void **)&TUnacquire, FUnacquire);
    }
}

void DetourGetDeviceData(LPDIRECTINPUTDEVICE8 device)
{
    if(!IsVFDetoured(device, kIdxGetDeviceData))
    {
        if(GetVFAddress(device, kIdxGetDeviceData, TGetDeviceData))
            DetourAttach((void **)&TGetDeviceData, FGetDeviceData);
    }
}

void DetourSetCooperativeLevel(LPDIRECTINPUTDEVICE8 device)
{
    if(!IsVFDetoured(device, kIdxSetCooperativeLevel))
    {
        if(GetVFAddress(device, kIdxSetCooperativeLevel, TSetCooperativeLevel))
            DetourAttach((void **)&TSetCooperativeLevel, FSetCooperativeLevel);
    }
}

void DetourGetDeviceState(LPDIRECTINPUTDEVICE8 device)
{
    if(!IsVFDetoured(device, kIdxGetDeviceState))
    {
        if(GetVFAddress(device, kIdxGetDeviceState, TGetDeviceState))
            DetourAttach((void **)&TGetDeviceState, FGetDeviceState);
    }
}

HRESULT (WINAPI FCreateDeviceA)(IDirectInput8 * self, REFGUID iid, 
    LPDIRECTINPUTDEVICE8 * device, LPUNKNOWN unknown)
{
    HRESULT hr;
    hr = TCreateDeviceA(self, iid, device, unknown);
    if(SUCCEEDED(hr) && device != NULL)
    {
        if(IsEqualGUID(iid, GUID_SysMouse_T) || 
            IsEqualGUID(iid, GUID_SysKeyboard))
        {
            auto i7e = *device;
            device_state[i7e] = kInit;

            DetourTransactionBegin();
            DetourUpdateThread(::GetCurrentThread());

            DetourSetCooperativeLevel(i7e);
            DetourAcquire(i7e);
            DetourUAcquire(i7e);
            DetourRelease(i7e);
            DetourGetDeviceData(i7e);
            DetourGetDeviceState(i7e);

            DetourTransactionCommit();
        }
    }
    return hr;
}

HRESULT (WINAPI FCreateDeviceW)(IDirectInput8 * self, REFGUID iid, 
    LPDIRECTINPUTDEVICE8 * device, LPUNKNOWN unknown)
{
    HRESULT hr;
    hr = TCreateDeviceW(self, iid, device, unknown);
    if(SUCCEEDED(hr) && device != NULL)
    {
        if(IsEqualGUID(iid, GUID_SysMouse_T) || 
            IsEqualGUID(iid, GUID_SysKeyboard))
        {
            auto i7e = *device;
            device_state[i7e] = kInit;

            DetourTransactionBegin();
            DetourUpdateThread(::GetCurrentThread());

            DetourSetCooperativeLevel(i7e);
            DetourAcquire(i7e);
            DetourUAcquire(i7e);
            DetourRelease(i7e);
            DetourGetDeviceData(i7e);
            DetourGetDeviceState(i7e);

            DetourTransactionCommit();
        }
    }
    return hr;
}

void DetourCreateDeviceA(LPVOID i7e)
{
    if(!IsVFDetoured(i7e, kIdxCreateDevice))
    {
        if(GetVFAddress(i7e, kIdxCreateDevice, TCreateDeviceA))
            DetourAttach((void **)&TCreateDeviceA, FCreateDeviceA);
    }
}

void DetourCreateDeviceW(LPVOID i7e)
{
    if(!IsVFDetoured(i7e, kIdxCreateDevice))
    {
        if(GetVFAddress(i7e, kIdxCreateDevice, TCreateDeviceW))
            DetourAttach((void **)&TCreateDeviceW, FCreateDeviceW);
    }
}

HRESULT WINAPI FDirectInput8Create(HINSTANCE hinst, 
    DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter)
{
    HRESULT hr;
    hr = TDirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);
    if(SUCCEEDED(hr) && ppvOut != NULL)
    {
        auto i7e = *ppvOut;
        DetourTransactionBegin();
        DetourUpdateThread(::GetCurrentThread());

        if(IsEqualGUID(riidltf, IID_IDirectInput8A_T))
        {
            DetourCreateDeviceA(i7e);
        }
        else if(IsEqualGUID(riidltf, IID_IDirectInput8W_T))
        {
            DetourCreateDeviceW(i7e);
        }
        DetourTransactionCommit();
    }
    return hr;
}

}

void DinputHooker::ToggleBlocked(bool block)
{
    using namespace dinputhooker;

    locker.Acquire();
    if(block)
    {
        Device2State::PairArray array = device_state.Items();
        for(auto iter = array.begin(); iter != array.end(); ++iter)
        {
            if(iter->second == kAcquire)
            {//当前是Acquire的要Unacquire
                TUnacquire(iter->first);
            }
        }     
    }
    else
    {
        Device2State::PairArray array = device_state.Items();
        for(auto iter = array.begin(); iter != array.end(); ++iter)
        {
            if(iter->second == kAcquire)
            {//当前是Acquire的 恢复时要保证Acquire
                TAcquire(iter->first);
            }
        }          
    }
    blocked = block;
    locker.Release();  
}
//////////////////////////////////////////////////////////////////////////
ncore::SpinLock DinputHooker::locker_;

void DinputHooker::Hook()
{
    using namespace dinputhooker;
    void * module = ::GetModuleHandle(L"dinput8.dll");
    if(module == 0)
        return;

    if(!IsModuleDetoured(module))
    {
        locker_.Acquire();
        if(!IsModuleDetoured(module))
        {
            FARPROC proc = 0;
            HMODULE mod = (HMODULE)module;
            proc = GetProcAddress(mod, "DirectInput8Create");
            TDirectInput8Create = (DirectInput8Create_T)proc;

            HANDLE curr_thread = ::GetCurrentThread();
            DetourTransactionBegin();
            DetourUpdateThread(curr_thread);
            if(TDirectInput8Create)
                DetourAttach(&(PVOID &)TDirectInput8Create, FDirectInput8Create);

            if(!DetourTransactionCommit())
                MarkModuleDetoured(module);

        }
        locker_.Release();
    }
}
}
}