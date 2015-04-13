//d3d11 headers will refer to windows header.
//without this macro that will lead to confilcit with winsock2 header
//we deinfe it outself.

#include "dxgi_hooker.h"
#include <DXGI.h>
#include <ncore\sys\spin_lock.h>
#include "detours\detours.h"
#include "detours\detours_ext.h"
#include "d3d11_canvas.h"
#include "d3d10_canvas.h"
#include "dxgi_common.h"
#include "utils.h"
#include "input_hooker.h"
#include "render_context.h"

namespace maku
{
namespace render
{
namespace dxgihooker
{
typedef LazyMap<IDXGISwapChain*, InternalCanvas*> SwapChain2Canvas;
SwapChain2Canvas swapchain_canvas;

ncore::SpinLock locker;
//
const IID IID_IDXGIFactory_T = {0x7b7166ec,0x21c7,0x44ae,
    0xb2,0x1a,0xc9,0xae,0x32,0x1a,0xe3,0x69};
const IID IID_IDXGIFactory1_T = {0x770aae78,0xf26f,0x4dba,
    0xa8,0x29,0x25,0x3c,0x83,0xd1,0xb3,0x87};
const IID IID_IDXGIDevice_T = {0x54ec77fa,0x1377,0x44e6,
    0x8c,0x32,0x88,0xfd,0x5f,0x44,0xc8,0x4c};
const IID IID_IDXGIAdapter_T = {0x2411e7e1,0x12ac,0x4ccf,
    0xbd,0x14,0x97,0x98,0xe8,0x53,0x4d,0xc0};
const IID IID_IDXGISwapChain_T = {0x310d36a0,0xd2e7,
    0x4c0a,0xaa,0x04,0x6a,0x9d,0x23,0xb8,0x88,0x6a};
//original function address
Release_T TRelease = 0;
CreateSwapChain_T TCreateSwapChain = 0;
SwapChainPresent_T TSwapChainPresent = 0;
SwapChainResizeBuffers_T TSwapChainResizeBuffers = 0;
SwapChainResizeTarget_T TSwapChainResizeTarget = 0;
CreateDXGIFactory_T TCreateDXGIFactory = NULL;
CreateDXGIFactory1_T TCreateDXGIFactory1 = NULL;

HRESULT WINAPI FCreateSwapChain(IDXGIFactory * self,
                                IUnknown * pDevice,
                                DXGI_SWAP_CHAIN_DESC * pDesc,
                                IDXGISwapChain ** ppSwapChain);

HRESULT WINAPI FSwapChainPresent(IDXGISwapChain * self,
                                 UINT SyncInterval,
                                 UINT Flags);

ULONG WINAPI FRelease(IUnknown * self);

HRESULT WINAPI FSwapChainResizeBuffers(IDXGISwapChain * self,
    UINT BufferCount,
    UINT Width,
    UINT Height,
    DXGI_FORMAT NewFormat,
    UINT SwapChainFlags);

HRESULT WINAPI FSwapChainResizeTarget(IDXGISwapChain * self,
    const DXGI_MODE_DESC * pNewTargetParameters);

HRESULT WINAPI FCreateDXGIFactory(
    _In_   REFIID riid,
    _Out_  void **ppFactory
    );

HRESULT WINAPI FCreateDXGIFactory1(
    _In_   REFIID riid,
    _Out_  void **ppFactory
    );

void OnPresenting(IDXGISwapChain * swapchain);

void InsertSwapchain(IDXGISwapChain * swapchain);

void DeleteSwapchain(IDXGISwapChain * swapchain);

InternalCanvas * GetCanvasBySwapchain(IDXGISwapChain * swapchain);
//////////////////////////////////////////////////////////////////////////
void DetourFactory(IDXGIFactory * i7e)
{
    if(IsVFDetoured(i7e, kIdxCreateSwapChain))
        return;
    if(GetVFAddress(i7e, kIdxCreateSwapChain, TCreateSwapChain))
        DetourAttach((void**)&TCreateSwapChain, FCreateSwapChain);
}

void DetourFactory1(IDXGIFactory1 * i7e)
{
    if(IsVFDetoured(i7e, kIdxCreateSwapChain))
        return;
    if(GetVFAddress(i7e, kIdxCreateSwapChain, TCreateSwapChain))
        DetourAttach((void**)&TCreateSwapChain, FCreateSwapChain);
}

void DetourSwapChainPresent(IDXGISwapChain * i7e)
{
    if(IsVFDetoured(i7e, kIdxSwapChainPresent))
        return;
    if(GetVFAddress(i7e, kIdxSwapChainPresent, TSwapChainPresent))
        DetourAttach((void**)&TSwapChainPresent, FSwapChainPresent);
}

void DetourSwapChainResizeBuffer(IDXGISwapChain * i7e)
{
    if(IsVFDetoured(i7e, kIdxSwapChainResizeBuffers))
        return;
    if(GetVFAddress(i7e, kIdxSwapChainResizeBuffers, TSwapChainResizeBuffers))
        DetourAttach((void**)&TSwapChainResizeBuffers, FSwapChainResizeBuffers);
}

void DetourSwapChainResizeTarget(IDXGISwapChain * i7e)
{
    if(IsVFDetoured(i7e, kIdxSwapChainResizeTarget))
        return;
    if(GetVFAddress(i7e, kIdxSwapChainResizeTarget, TSwapChainResizeTarget))
        DetourAttach((void**)&TSwapChainResizeTarget, FSwapChainResizeTarget);
}

void DetourRelease(IUnknown * i7e)//不HOOK Device 改为HOOK SwapChain
{
    if(IsVFDetoured(i7e, kIdxRelease))
        return;
    if(GetVFAddress(i7e, kIdxRelease, TRelease))
        DetourAttach((void**)&TRelease, FRelease);
}

/*********************************************
 FCreateDXGIFactory

 HOOK IDXGIFactory::CreateSwapChain
 **********************************************/
HRESULT WINAPI FCreateDXGIFactory(
    _In_   REFIID riid,
    _Out_  void **ppFactory
    )
{
    HRESULT hr = TCreateDXGIFactory(riid, ppFactory);
    IDXGIFactory * i7e = (IDXGIFactory*)(*ppFactory);

    DetourTransactionBegin();
    DetourUpdateThread(::GetCurrentThread());
    DetourFactory(i7e);    
    DetourTransactionCommit();
    return hr;
}

/*********************************************
 FCreateDXGIFactory1

 HOOK IDXGIFactory1::CreateSwapChain
 **********************************************/
HRESULT WINAPI FCreateDXGIFactory1(
    _In_   REFIID riid,
    _Out_  void **ppFactory
    )
{
    HRESULT hr = TCreateDXGIFactory1(riid, ppFactory);
    IDXGIFactory1 * i7e = (IDXGIFactory1*)(*ppFactory);
    
    DetourTransactionBegin();
    DetourUpdateThread(::GetCurrentThread());
    DetourFactory1(i7e);
    DetourTransactionCommit();
    return hr;
}

/*********************************************
 FCreateSwapChain

 HOOK IDXGISwapChain::Present
      IDXGISwapChain::ResizeBuffer
      IDXGISwapChain::ResizeTarget
      IDXGISwapChain::Release
 **********************************************/
HRESULT WINAPI FCreateSwapChain(IDXGIFactory * self,
    IUnknown *pDevice,
    DXGI_SWAP_CHAIN_DESC *pDesc,
    IDXGISwapChain **ppSwapChain)
{
    if(TCreateSwapChain == 0)
        return S_FALSE;

    HRESULT hr = TCreateSwapChain(self, pDevice, pDesc, ppSwapChain);

    if(S_OK == hr)
    {
        DetourTransactionBegin();
        DetourUpdateThread(::GetCurrentThread());
        if(ppSwapChain !=0)
        {
            DetourSwapChainPresent(*ppSwapChain);
            DetourSwapChainResizeBuffer(*ppSwapChain);
            DetourSwapChainResizeTarget(*ppSwapChain);
            DetourRelease(*ppSwapChain);
            InsertSwapchain(*ppSwapChain);
        }
        DetourTransactionCommit();
    }
    return hr;
}

/*********************************************
 FSwapChainResizeBuffers
 后备缓冲区改变 重新创建Canvas
 **********************************************/
HRESULT WINAPI FSwapChainResizeBuffers(IDXGISwapChain * self,
    UINT BufferCount,
    UINT Width,
    UINT Height,
    DXGI_FORMAT NewFormat,
    UINT SwapChainFlags)
{
    HRESULT hr = S_FALSE;
    if (TSwapChainResizeBuffers)
    {
        InternalCanvas * internal_canvas = GetCanvasBySwapchain(self);
        if(internal_canvas)
            DeleteSwapchain(self);

        hr = TSwapChainResizeBuffers(self, BufferCount, Width,
            Height, NewFormat, SwapChainFlags);

        if(internal_canvas)
            InsertSwapchain(self);
    }
    return hr;
}

/*********************************************
 FRelease
 Swapchain释放时 释放对应的Canvas
 **********************************************/
ULONG WINAPI FRelease(IUnknown * self)
{
    ULONG ret = 0;
    IDXGISwapChain* swapchain = NULL;
    self->QueryInterface(IID_IDXGISwapChain_T, (void**)&swapchain);
    if (swapchain)
    {
        ret = TRelease(swapchain);
        ret = TRelease(self);
        if (ret == 0)
        {
            InternalCanvas * internal_canvas = GetCanvasBySwapchain(swapchain);
            if (internal_canvas)
            {
                DeleteSwapchain(swapchain);
                ret = 0;
            }
        }  
    }
    else
        ret = TRelease(self);
    return ret;
}

/*********************************************
 FSwapChainResizeTarget
 后备缓冲区改变 重新创建Canvas
 **********************************************/
HRESULT WINAPI FSwapChainResizeTarget(IDXGISwapChain *self,
    const DXGI_MODE_DESC *pNewTargetParameters)
{
    HRESULT hr = S_FALSE;
    if (TSwapChainResizeTarget)
    {
        InternalCanvas * internal_canvas = GetCanvasBySwapchain(self);
        if(internal_canvas)
            DeleteSwapchain(self);

        hr = TSwapChainResizeTarget(self, pNewTargetParameters);

        if(internal_canvas)
            InsertSwapchain(self);
    }
    return hr;
}

/*********************************************
 FSwapChainPresent
 调用自己的绘制例程
 **********************************************/
HRESULT WINAPI FSwapChainPresent(IDXGISwapChain *self,
    UINT SyncInterval,
    UINT Flags)
{
    if(TSwapChainPresent)
    {
        OnPresenting(self);
        HRESULT  hr = TSwapChainPresent(self, SyncInterval, Flags);
        return hr;
    }

    return S_FALSE;
}

bool GetSourceRectAndWindowed(
    IDXGISwapChain * swapchain, 
    RECT & source_rect,
    bool & windowed)
{
    DXGI_SWAP_CHAIN_DESC swap_desc;
    ZeroMemory(&swap_desc, sizeof(swap_desc));
    if(FAILED(swapchain->GetDesc(&swap_desc)))
        return false;
    //
    uint32_t width = swap_desc.BufferDesc.Width;
    uint32_t height = swap_desc.BufferDesc.Height;
    //set user32 params
    windowed = swap_desc.Windowed ? true : false;

    source_rect.left = source_rect.top = 0;
    source_rect.right = width;
    source_rect.bottom = height;
    return true;
}

void GetDestRect(HWND hwnd, bool windowed, RECT & des_rect)
{
    if(windowed)
        GetClientRect(hwnd, &des_rect);
    else
        GetWindowRect(hwnd, &des_rect);
}

HWND GetDeviceHWND(IDXGISwapChain * swapchain)
{
    HWND hwnd = NULL;
    DXGI_SWAP_CHAIN_DESC swap_desc;
    ZeroMemory(&swap_desc, sizeof(swap_desc));
    if(FAILED(swapchain->GetDesc(&swap_desc)))
        return hwnd;
    //set window handle
    hwnd = swap_desc.OutputWindow;

    return hwnd;
}

void OnPresenting(IDXGISwapChain * swapchain)
{
    RECT source_rect, des_rect;
    bool windowed = true;
    HWND game_hwnd = NULL;

    game_hwnd = GetDeviceHWND(swapchain);
    HWND cur_hwnd = InputHooker::GetCurrentHWND();

    if(game_hwnd != cur_hwnd && !IsChild(cur_hwnd, game_hwnd))
        return;

    if (!GetSourceRectAndWindowed(swapchain, source_rect, windowed))
        return;

    RenderContext::Get()->UpdateStatus(source_rect.right - source_rect.left,
        source_rect.bottom - source_rect.top);

    if(RenderContext::Get()->IsPresent())
    {
        InternalCanvas* internal_canvas = GetCanvasBySwapchain(swapchain);
        if (internal_canvas)
        {
            GetDestRect(game_hwnd, windowed, des_rect);
            InputHooker::SetWindowed(windowed);
            InputHooker::SetTransRect(des_rect, source_rect);

            internal_canvas->SaveStatus();
            Canvas* canvas = internal_canvas;
            RenderContext::Get()->Draw(canvas);
            internal_canvas->RestoreStatus();
        }
    }
}

void InsertSwapchain(IDXGISwapChain * swapchain)
{
    ID3D10Device * device10 = NULL;
    InternalCanvas * internal_canvas = NULL;
    swapchain->GetDevice(__uuidof(ID3D10Device), (void**)&device10);
    if (NULL != device10)
    {
        internal_canvas = new D3D10Canvas(swapchain);
        device10->Release();
    }
    else
        internal_canvas = new D3D11Canvas(swapchain);

    locker.Acquire();
    swapchain_canvas[swapchain] = internal_canvas;
    locker.Release();
}

void DeleteSwapchain(IDXGISwapChain * swapchain)
{
    InternalCanvas * internal_canvas = NULL;
    locker.Acquire();
    if(swapchain_canvas.Has(swapchain))
    {
        internal_canvas = swapchain_canvas[swapchain];
        swapchain_canvas.Remove(swapchain);
    }
    locker.Release();

    if(internal_canvas)
        delete internal_canvas;
}

InternalCanvas * GetCanvasBySwapchain(IDXGISwapChain * swapchain)
{
    InternalCanvas * canvas = NULL;

    locker.Acquire();
    if(swapchain_canvas.Has(swapchain))
    {
        canvas = swapchain_canvas[swapchain];
    }
    locker.Release();

    return canvas;
}




}
//////////////////////////////////////////////////////////////////////////
ncore::SpinLock DXGIHooker::locker_;
void DXGIHooker::Hook()
{
    using namespace dxgihooker;

    wchar_t dxgi_path[MAX_PATH] =  {0};
    if( 0 == ::GetSystemDirectory(dxgi_path, sizeof(dxgi_path)) )
        return;
    wcscat_s(dxgi_path, L"\\dxgi.dll");

    void * module = ::GetModuleHandle(dxgi_path);

    if(module == 0)
        return;

    //see also: kernel_hooker.cpp
    if(!IsModuleDetoured(module))
    {
        if(!locker_.TryAcquire())
            return;
        if(!IsModuleDetoured(module))
        {
            FARPROC proc = 0;
            HMODULE mod = (HMODULE)module;
            //hook
            proc = GetProcAddress(mod, "CreateDXGIFactory");
            TCreateDXGIFactory  = (CreateDXGIFactory_T)proc;

            proc = GetProcAddress(mod, "CreateDXGIFactory1");
            TCreateDXGIFactory1 = (CreateDXGIFactory1_T)proc;

            HANDLE curr_thread = GetCurrentThread();
            DetourTransactionBegin();
            DetourUpdateThread(curr_thread);
            if(TCreateDXGIFactory)
                DetourAttach(&(PVOID&)TCreateDXGIFactory, FCreateDXGIFactory);
            if(TCreateDXGIFactory1)
                DetourAttach(&(PVOID&)TCreateDXGIFactory1, FCreateDXGIFactory1);
            //if every thing is ok. mark this module detoured.
            if(!DetourTransactionCommit())
                MarkModuleDetoured(module);
        }
        locker_.Release();
    } 
}
}
}
