//without this macro that will lead to confilcit with winsock2 header
//detour rely on windows header

#include "d3d9_hooker.h"
#include "detours\detours.h"
#include "detours\detours_ext.h"
#include "render_context.h"
#include "utils.h"
#include "input_hooker.h"
#include "d3d9_canvas.h"
#include "d3d9_common.h"

namespace maku
{
namespace render
{
namespace d3d9hooker
{
typedef LazyMap<IDirect3DDevice9*, D3D9Canvas*> Device92Canvas;
Device92Canvas device9_canvas;
ncore::SpinLock locker;
//original function address
Direct3DCreate9_T TDirect3DCreate9 = 0;
Direct3DCreate9Ex_T TDirect3DCreate9Ex = 0;
CreateDevice_T TCreateDevice = 0;//IDirect3D9中的CreateDevice
CreateDevice_T TCreateDeviceFromEx = 0;//IDirect3D9Ex中的CreateDevice
CreateDeviceEx_T TCreateDeviceEx = NULL;
Present_T TPresent = 0;
PresentEx_T TPresentEx = NULL;
Release_T TRelease = NULL;
Reset_T TReset = NULL;
ResetEx_T TResetEx = NULL;
GetSwapChain_T TGetSwapChain = 0;
SwapChainPresent_T TSwapChainPresent = 0;
CreateAdditionalSwapChain_T TCreateAdditionalSwapChain = 0;

//detour function declare
IDirect3D9 * WINAPI FDirect3DCreate9(UINT SDKVersion);

HRESULT WINAPI FDirect3DCreate9Ex(UINT SDKVersion, IDirect3D9Ex ** ptr);

ULONG WINAPI FRelease(IUnknown * self);

HRESULT WINAPI FReset(
    IDirect3DDevice9 * self, 
    D3DPRESENT_PARAMETERS * pPresentationParameters);

HRESULT WINAPI FResetEx(
    IDirect3DDevice9Ex * self,
    D3DPRESENT_PARAMETERS *pPresentationParameters,
    D3DDISPLAYMODEEX *pFullscreenDisplayMode
    );

HRESULT WINAPI FCreateDevice(
    IDirect3D9 * self, 
    UINT Adapter,
    D3DDEVTYPE DeviceType,
    HWND hFocusWindow,
    DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS * pParams,
    IDirect3DDevice9 ** ppDeviceIntf);

HRESULT WINAPI FCreateDeviceEx(
    IDirect3D9Ex * self,
    UINT Adapter,
    D3DDEVTYPE DeviceType,
    HWND hFocusWindow,
    DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS* pPresentationParameters,
    D3DDISPLAYMODEEX* pFullscreenDisplayMode,
    IDirect3DDevice9Ex** ppReturnedDeviceInterface
    );

HRESULT WINAPI FCreateDeviceFromEx(
    IDirect3D9Ex * self,
    UINT Adapter,
    D3DDEVTYPE DeviceType,
    HWND hFocusWindow,
    DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS* pPresentationParameters,
    IDirect3DDevice9** ppReturnedDeviceInterface);

HRESULT WINAPI FGetSwapChain(
    IDirect3DDevice9 * self, 
    UINT iSwapChain,
    IDirect3DSwapChain9 ** pSwapChain);

HRESULT WINAPI FCreateAdditionalSwapChain(
    IDirect3DDevice9 * self, 
    D3DPRESENT_PARAMETERS * pPresentationParameters,
    IDirect3DSwapChain9 ** pSwapChain);

HRESULT WINAPI FPresent(
    IDirect3DDevice9 * self, 
    CONST RECT * pSourceRect,
    CONST RECT * pDestRect,
    HWND hDestWindowOverride,
    CONST RGNDATA * pDirtyRegion);

HRESULT WINAPI FPresentEx(
    IDirect3DDevice9Ex * self,
    const RECT *pSourceRect,
    const RECT *pDestRect,
    HWND hDestWindowOverride,
    const RGNDATA *pDirtyRegion,
    DWORD dwFlags
    );

HRESULT WINAPI FSwapChainPresent(
    IDirect3DSwapChain9 * self,
    RECT * pSourceRect,
    CONST RECT * pDestRect,
    HWND hDestWindowOverride,
    CONST RGNDATA * pDirtyRegion,
    DWORD dwFlags);

void OnPresenting(
    IDirect3DDevice9 * device, 
    CONST RECT * pSourceRect,
    CONST RECT * pDestRect,
    HWND hDestWindowOverride,
    CONST RGNDATA * pDirtyRegion);

void OnSwapChainPresenting( 
         IDirect3DSwapChain9 * self,
         CONST RECT * pSourceRect,
         CONST RECT * pDestRect,
         HWND hDestWindowOverride,
         CONST RGNDATA * pDirtyRegion);

void InsertDevice(IDirect3DDevice9 * device);

void DeleteDevice(IDirect3DDevice9 * device);

D3D9Canvas * GetCanvasByDevice(IDirect3DDevice9 * device);
//////////////////////////////////////////////////////////////////////////
void DetourPresent(IDirect3DDevice9 * i7e) //挂钩Present函数
{
    //Present
    if(IsVFDetoured(i7e, kIdxPresent))
        return;

    if(GetVFAddress(i7e, kIdxPresent, TPresent))
        DetourAttach(&(PVOID&)TPresent, FPresent);
}

void DetourPresentEx(IDirect3DDevice9Ex * i7e)
{
    if(IsVFDetoured(i7e, kIdxPresentEx))
        return;

    if(GetVFAddress(i7e, kIdxPresentEx, TPresentEx))
        DetourAttach(&(PVOID&)TPresentEx, FPresentEx);
}

void DetourResetEx(IDirect3DDevice9Ex * i7e)
{
    if(IsVFDetoured(i7e, kIdxResetEx))
        return;

    if(GetVFAddress(i7e, kIdxResetEx, TResetEx))
        DetourAttach(&(PVOID&)TResetEx, FResetEx);
}

void DetourRelease(IDirect3DDevice9 * i7e) //挂钩Release函数
{
    //Release
    if(IsVFDetoured(i7e, kIdxRelease))
        return;
    if(GetVFAddress(i7e, kIdxRelease, TRelease))
        DetourAttach((void**)&TRelease, FRelease);
}

void DetourReset(IDirect3DDevice9 * i7e) //挂钩Reset函数
{
    if(IsVFDetoured(i7e, kIdxReset))
        return;
    if(GetVFAddress(i7e, kIdxReset, TReset))
        DetourAttach((void**)&TReset, FReset);
}

void DetourGetSwapChain(IDirect3DDevice9 * i7e)
{
    //GetSwapChain
    if(IsVFDetoured(i7e, kIdxGetSwapChain))
        return;

    if(GetVFAddress(i7e, kIdxGetSwapChain, TGetSwapChain))
        DetourAttach((void**)&TGetSwapChain, FGetSwapChain);
}

void DetourCreateAdditionalSwapChain(IDirect3DDevice9 * i7e)
{
    if(IsVFDetoured(i7e, kIdxCreateAdditionalSwapChain))
        return;

    if(GetVFAddress(i7e, kIdxCreateAdditionalSwapChain,
        TCreateAdditionalSwapChain))

        DetourAttach((void**)&TCreateAdditionalSwapChain, 
        FCreateAdditionalSwapChain);
}

void DetourSwapChainPresent(IDirect3DSwapChain9 * i7e)
{
    if(IsVFDetoured(i7e, kIdxSwapChainPresent))
        return;

    if(GetVFAddress(i7e, kIdxSwapChainPresent, TSwapChainPresent))
        DetourAttach((void**)&TSwapChainPresent, FSwapChainPresent);
}

/*********************************************
 FDirect3DCreate9
 Hook IDirect3D9::CreateDevice
 **********************************************/
//Implement
IDirect3D9 * WINAPI FDirect3DCreate9(UINT SDKVersion)
{
    auto i7e = TDirect3DCreate9(SDKVersion);
    if(i7e)
    {
        DetourTransactionBegin();
        DetourUpdateThread(::GetCurrentThread());

        if(!IsVFDetoured(i7e, kIdxCreateDevice))
        {
            //Detach hook before Attach it again.
            if((CreateDevice_T)GetVFAddress(i7e, kIdxCreateDevice, TCreateDevice))
                DetourAttach((void**)&TCreateDevice, FCreateDevice);
        }
        DetourTransactionCommit();
    }
    return i7e;
}


/*********************************************
 FDirect3DCreate9Ex
 Hook IDirect3D9Ex::CreateDeviceEx
 **********************************************/
HRESULT WINAPI FDirect3DCreate9Ex(UINT SDKVersion, IDirect3D9Ex** ptr)
{
    HRESULT hr = TDirect3DCreate9Ex(SDKVersion, ptr);
    if (ptr)
    {
        auto i7e = *ptr;
        DetourTransactionBegin();
        DetourUpdateThread(::GetCurrentThread());

        if(!IsVFDetoured(i7e, kIdxCreateDeviceEx))
        {
            //Detach hook before Attach it again.
            if(GetVFAddress(i7e, kIdxCreateDeviceEx, TCreateDeviceEx))
                DetourAttach((void**)&TCreateDeviceEx, FCreateDeviceEx);
        }

        if(!IsVFDetoured(i7e, kIdxCreateDevice))
        {
            if(GetVFAddress(i7e, kIdxCreateDevice, TCreateDeviceFromEx))
                DetourAttach((void**)&TCreateDeviceFromEx, 
                             FCreateDeviceFromEx);
        }

        DetourTransactionCommit();
    }
    return hr;
}

/*********************************************
 FCreateDevice
 Hook IDirect3DDevice9::Release
      IDirect3DDevice9::Reset
      IDirect3DDevice9::Present
      IDirect3DDevice9::GetSwapChain
      IDirect3DDevice9::CreateAdditionalSwapChain
      创建与Device对应的Canvas
 **********************************************/
HRESULT WINAPI FCreateDevice(IDirect3D9 * self, 
                             UINT Adapter,
                             D3DDEVTYPE DeviceType,
                             HWND hFocusWindow,
                             DWORD BehaviorFlags,
                             D3DPRESENT_PARAMETERS* pParams,
                             IDirect3DDevice9** ppDeviceIntf)
{
    if(TCreateDevice)
    {
        IDirect3DDevice9 * i7e = 0;
         auto hr = TCreateDevice(self, Adapter, DeviceType, hFocusWindow, 
                                 BehaviorFlags, pParams, ppDeviceIntf);
         if(ppDeviceIntf)
             i7e = *ppDeviceIntf;
         if(hr == S_OK && i7e)
         {
             InsertDevice(i7e);
             DetourTransactionBegin();
             DetourUpdateThread(::GetCurrentThread());

             DetourRelease(i7e);
             DetourReset(i7e);
             DetourPresent(i7e);
             DetourGetSwapChain(i7e);
             DetourCreateAdditionalSwapChain(i7e);

             DetourTransactionCommit();   
         }
         return hr;
    }
    else
    {
        return S_FALSE;
    }
}

/*********************************************
 FCreateDeviceEx
 Hook IDirect3DDevice9Ex::Release
      IDirect3DDevice9Ex::Reset
      IDirect3DDevice9Ex::Present
      IDirect3DDevice9Ex::ResetEx
      IDirect3DDevice9Ex::PresentEx
      IDirect3DDevice9Ex::GetSwapChain
      IDirect3DDevice9Ex::CreateAdditionalSwapChain

      创建与Device对应的Canvas
 **********************************************/
HRESULT WINAPI FCreateDeviceEx(
    IDirect3D9Ex * self,
    UINT Adapter,
    D3DDEVTYPE DeviceType,
    HWND hFocusWindow,
    DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS* pPresentationParameters,
    D3DDISPLAYMODEEX* pFullscreenDisplayMode,
    IDirect3DDevice9Ex** ppReturnedDeviceInterface
    )
{
    if (TCreateDeviceEx)
    {
        HRESULT hr = TCreateDeviceEx(self, Adapter, DeviceType, hFocusWindow,
            BehaviorFlags, pPresentationParameters, pFullscreenDisplayMode, 
            ppReturnedDeviceInterface);
        IDirect3DDevice9Ex * i7e = *ppReturnedDeviceInterface;
        if (SUCCEEDED(hr) && i7e)
        {
            InsertDevice(i7e);
            DetourTransactionBegin();
            DetourUpdateThread(::GetCurrentThread());

            DetourRelease(i7e);
            DetourReset(i7e);
            DetourPresent(i7e);
            DetourResetEx(i7e);
            DetourPresentEx(i7e);
            DetourGetSwapChain(i7e);
            DetourCreateAdditionalSwapChain(i7e);

            DetourTransactionCommit(); 
        }
        return hr;
    } 
    else
    {
        return S_FALSE;
    }
}

HRESULT WINAPI FCreateDeviceFromEx(
    IDirect3D9Ex * self,
    UINT Adapter,
    D3DDEVTYPE DeviceType,
    HWND hFocusWindow,
    DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS* pPresentationParameters,
    IDirect3DDevice9** ppReturnedDeviceInterface)
{
    if(TCreateDeviceFromEx)
    {
        IDirect3DDevice9 * i7e = 0;
        auto hr = TCreateDeviceFromEx(self, Adapter, DeviceType, hFocusWindow, 
            BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
        if(ppReturnedDeviceInterface)
            i7e = *ppReturnedDeviceInterface;
        if(hr == S_OK && i7e)
        {
            InsertDevice(i7e);
            DetourTransactionBegin();
            DetourUpdateThread(::GetCurrentThread());

            DetourRelease(i7e);
            DetourReset(i7e);
            DetourPresent(i7e);
            DetourGetSwapChain(i7e);
            DetourCreateAdditionalSwapChain(i7e);

            DetourTransactionCommit();   
        }
        return hr;
    }
    else
    {
        return S_FALSE;
    }
}

void GetDestRect(HWND hwnd, CONST RECT * pDestRect,
    bool windowed, RECT & des_rect)
{
    if (NULL == pDestRect)
    {   
        if(windowed)
            GetClientRect(hwnd, &des_rect);
        else
            GetWindowRect(hwnd, &des_rect);
    }
    else
    {
        des_rect = *pDestRect;
    }
}

void OnCanvasPresenting(IDirect3DDevice9 * device, 
    IDirect3DSurface9 * target,
    RECT & source, RECT & dest,
    bool windowed, HWND game_hwnd)
{
    HWND cur_hwnd = InputHooker::GetCurrentHWND();
    if (game_hwnd != cur_hwnd && !IsChild(cur_hwnd, game_hwnd))
        return;

    RenderContext::Get()->UpdateStatus(source.right - source.left,
        source.bottom - source.top);

    if(RenderContext::Get()->IsPresent())
    {
        D3D9Canvas * canvas = GetCanvasByDevice(device);
        if(canvas)
        {
            InputHooker::SetWindowed(windowed);
            InputHooker::SetTransRect(dest, source);
            canvas->UpdateRenderTarget(target, source);
            canvas->SaveStatus();
            RenderContext::Get()->Draw(canvas);
            canvas->RestoreStatus();
        }
    }
}

void OnPresenting(IDirect3DDevice9 * device, CONST RECT * pSourceRect,
    CONST RECT * pDestRect, HWND hDestWindowOverride,
    CONST RGNDATA * pDirtyRegion)
{
    D3DDEVICE_CREATION_PARAMETERS dcp;
    HRESULT hr = device->GetCreationParameters(&dcp);    
    if(FAILED(hr))
        return;
    //set window handle
    HWND game_hwnd = dcp.hFocusWindow;
    if(hDestWindowOverride)
        game_hwnd = hDestWindowOverride;

    IDirect3DSurface9 * target = 0;
    device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &target);
    if(target == 0)
        return;
    target->Release();

    LPDIRECT3DSWAPCHAIN9 swap_chain = 0;
    hr = device->GetSwapChain(0, &swap_chain); 
    if(FAILED(hr))
        return;
    swap_chain->Release();

    D3DPRESENT_PARAMETERS desc;
    hr = swap_chain->GetPresentParameters(&desc); 
    if(FAILED(hr))
        return;
    //little_inferno dcp.hFocusWindow 和 hDestWindowOverride 都为0
    if(game_hwnd == 0)
        game_hwnd = desc.hDeviceWindow;

    RECT source = {0, 0, desc.BackBufferWidth, desc.BackBufferHeight};
    //若sourcerect有值，则求sourcerect和backbuffer的交集矩形 
    if (pSourceRect != NULL)
    { 
        RECT tmp_source = source;
        IntersectRect(&source, &tmp_source, pSourceRect);
    }
    bool windowed = desc.Windowed ? true : false;

    RECT dest = {0};
    GetDestRect(game_hwnd, pDestRect, windowed, dest);

    OnCanvasPresenting(device, target, source, dest, windowed, game_hwnd);
}

void OnSwapChainPresenting( 
    IDirect3DSwapChain9 * self,
    CONST RECT * pSourceRect,
    CONST RECT * pDestRect,
    HWND hDestWindowOverride,
    CONST RGNDATA * pDirtyRegion)
{
    D3DPRESENT_PARAMETERS param;
    auto hr = self->GetPresentParameters(&param);
    if(FAILED(hr))
        return;
    HWND game_hwnd = param.hDeviceWindow;
    if(hDestWindowOverride)
        game_hwnd = hDestWindowOverride;

    IDirect3DDevice9 * device = 0;
    self->GetDevice(&device);
    if(!device)
        return;
    device->Release();

    IDirect3DSurface9 * target = 0;
    self->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &target);
    if(target == 0)
        return;
    target->Release();

    RECT source = {0, 0, param.BackBufferWidth, param.BackBufferHeight};
    if(pSourceRect)
    {
        RECT tmp_source = source;
        IntersectRect(&source, &tmp_source, pSourceRect);
    }

    bool windowed = param.Windowed ? true : false;

    RECT dest = {0};
    GetDestRect(game_hwnd, pDestRect, windowed, dest);

    OnCanvasPresenting(device, target, source, dest, windowed, game_hwnd);
}

/*********************************************
 FReset
 在device Reset前 释放纹理 保证device Reset成功
 **********************************************/
HRESULT WINAPI FReset(IDirect3DDevice9 * self, 
    D3DPRESENT_PARAMETERS * pPresentationParameters)
{
    HRESULT hr = S_FALSE;
    D3D9Canvas* d3d9_canvas = GetCanvasByDevice(self);
    if (d3d9_canvas)
        DeleteDevice(self);
    if(TReset)
    {
        hr = TReset(self, pPresentationParameters);

        if (S_OK == hr)
            InsertDevice(self);
    }
    return hr;
}
//ResetEx
HRESULT WINAPI FResetEx(
    IDirect3DDevice9Ex * self,
    D3DPRESENT_PARAMETERS *pPresentationParameters,
    D3DDISPLAYMODEEX *pFullscreenDisplayMode
    )
{
    HRESULT hr =S_FALSE;
    D3D9Canvas* d3d9_canvas = GetCanvasByDevice(self);
    if (d3d9_canvas)
        DeleteDevice(self);
    if (TResetEx)
    {
        hr = TResetEx(self, pPresentationParameters,
            pFullscreenDisplayMode);

        if(S_OK == hr)
            InsertDevice(self);
    }
    return hr;
}

/*********************************************
 FRelease
 Release的对象是Device时 Device与Canvas的引用计数
 相等时 删除Canvas
 **********************************************/
ULONG WINAPI FRelease(IUnknown * self)
{
    ULONG ret = 0;
    IDirect3DDevice9* device = NULL;
    self->QueryInterface(__uuidof(IDirect3DDevice9), (void**)&device);
    if (device)
    { 
        ret = TRelease(device); // 还原计数
        ret = TRelease(self);

        D3D9Canvas* d3d9_canvas = GetCanvasByDevice(device);
        if (d3d9_canvas)
        {
            uint32_t count = d3d9_canvas->GetCountRef();
            if (count == ret)
            {
                DeleteDevice(device);
                ret = 0;
            }
        }
    }
    else 
        ret = TRelease(self);
    return ret;
}

/*********************************************
 FGetSwapChain
 Hook IDirect3DSwapChain9::Present
 **********************************************/
HRESULT WINAPI FGetSwapChain(IDirect3DDevice9 * self, 
                             UINT iSwapChain,
                             IDirect3DSwapChain9 ** pSwapChain)
{
    IDirect3DSwapChain9 * i7e = 0;
    if(TGetSwapChain)
    {
        HRESULT hr = S_FALSE;
        hr = TGetSwapChain(self, iSwapChain, pSwapChain);
        if(pSwapChain)
            i7e = *pSwapChain;
        if(hr == S_OK && i7e)
        {
            DetourTransactionBegin();
            DetourUpdateThread(::GetCurrentThread());
            DetourSwapChainPresent(i7e);
            DetourTransactionCommit();
        }
        return hr;
    }
    return S_FALSE;
}

void InsertDevice(IDirect3DDevice9 * device)
{
    D3D9Canvas * d3d9_canvas = new D3D9Canvas(device);
    locker.Acquire();
    device9_canvas[device] = d3d9_canvas;
    locker.Release();
}

void DeleteDevice(IDirect3DDevice9 * device)
{
    D3D9Canvas * d3d9_canvas = NULL;
    locker.Acquire();
    if(device9_canvas.Has(device))
    {
        d3d9_canvas = device9_canvas[device];
        device9_canvas.Remove(device);
    }
    locker.Release();
    if(d3d9_canvas)
        delete d3d9_canvas;
}

D3D9Canvas* GetCanvasByDevice(IDirect3DDevice9 * device)
{
    D3D9Canvas * canvas = NULL;

    locker.Acquire();
    if(device9_canvas.Has(device))
        canvas = device9_canvas[device];
    locker.Release();

    return canvas;
}

/*********************************************
 FCreateAdditionalSwapChain
 Hook IDirect3DSwapChain9::Present
 **********************************************/
HRESULT WINAPI FCreateAdditionalSwapChain(IDirect3DDevice9 * self, 
                        D3DPRESENT_PARAMETERS* pPresentationParameters,
                        IDirect3DSwapChain9 ** pSwapChain)
{
    IDirect3DSwapChain9 * i7e = 0;
    if(TCreateAdditionalSwapChain)
    {
        HRESULT hr = S_FALSE;
        hr = TCreateAdditionalSwapChain(self, pPresentationParameters, 
            pSwapChain);
        if(pSwapChain)
            i7e = *pSwapChain;
        if( hr == S_OK && i7e)
        {
            DetourTransactionBegin();
            DetourUpdateThread(::GetCurrentThread());
            DetourSwapChainPresent(i7e);
            DetourTransactionCommit();
        }
        return hr;
    }
    return S_FALSE; 
}

/*********************************************
 FPresent
 调用自己的绘制例程
 **********************************************/
HRESULT WINAPI FPresent(IDirect3DDevice9 * self, 
                        CONST RECT * pSourceRect,
                        CONST RECT * pDestRect,
                        HWND hDestWindowOverride,
                        CONST RGNDATA * pDirtyRegion)
{
    if(TPresent)
    {
        HRESULT hr = S_FALSE;
        OnPresenting(self, pSourceRect, pDestRect, 
            hDestWindowOverride, pDirtyRegion);

        hr = TPresent(self, pSourceRect, pDestRect, hDestWindowOverride,
            pDirtyRegion);		
        return hr;
    }
    return S_FALSE;
}
//PresentEx
/*********************************************
 FPresentEx
 调用自己的绘制例程
 **********************************************/
HRESULT WINAPI FPresentEx(
    IDirect3DDevice9Ex * self,
    const RECT *pSourceRect,
    const RECT *pDestRect,
    HWND hDestWindowOverride,
    const RGNDATA *pDirtyRegion,
    DWORD dwFlags
    )
{
    HRESULT hr = S_FALSE;
    OnPresenting(self, pSourceRect, pDestRect, hDestWindowOverride,
        pDirtyRegion);

    hr = TPresentEx(self, pSourceRect, pDestRect,
        hDestWindowOverride, pDirtyRegion, dwFlags);
    return hr;
}

/*********************************************
 FSwapChainPresent
 调用自己的绘制例程
 **********************************************/
HRESULT WINAPI FSwapChainPresent(IDirect3DSwapChain9 * self,
                                 RECT* pSourceRect,
                                 CONST RECT* pDestRect,
                                 HWND hDestWindowOverride,
                                 CONST RGNDATA* pDirtyRegion,
                                 DWORD dwFlags)
{
    HRESULT hr = S_FALSE;
    if(TSwapChainPresent)
    {
        OnSwapChainPresenting(self, pSourceRect, pDestRect, 
                hDestWindowOverride, pDirtyRegion);

        hr = TSwapChainPresent(self, pSourceRect, pDestRect, 
                               hDestWindowOverride, pDirtyRegion, dwFlags);
    }
    return hr;
}

}

/*/////////////////////////////////////////////////////////////////////////////
//D3D9Hooker
/////////////////////////////////////////////////////////////////////////////*/
ncore::SpinLock D3D9Hooker::locker_;

void D3D9Hooker::Hook()
{
    using namespace d3d9hooker;
    wchar_t d3d9_path[MAX_PATH] =  {0};
    if( 0 == ::GetSystemDirectory(d3d9_path, sizeof(d3d9_path)) )
        return;
    wcscat_s(d3d9_path, L"\\d3d9.dll");

    void * module = ::GetModuleHandle(d3d9_path);
    if(module == 0)
        return;

    
    //see also: kernel_hooker.cpp
    if(!IsModuleDetoured(module))
    {
        if(!locker_.TryAcquire())
            return;
        if(!IsModuleDetoured(module)) //防止多线程重复载入
        {
            FARPROC proc = 0;
            HMODULE mod = (HMODULE)module;

            proc = GetProcAddress(mod, "Direct3DCreate9");
            TDirect3DCreate9  = (Direct3DCreate9_T)proc;
            proc = GetProcAddress(mod, "Direct3DCreate9Ex");
            TDirect3DCreate9Ex = (Direct3DCreate9Ex_T)proc;

            DetourTransactionBegin();
            DetourUpdateThread(::GetCurrentThread());
            if(TDirect3DCreate9)
            DetourAttach(&(PVOID&)TDirect3DCreate9, FDirect3DCreate9);
            //early version d3d9.dll does not include this iterface.
            if(TDirect3DCreate9Ex)
                DetourAttach(&(PVOID&)TDirect3DCreate9Ex, FDirect3DCreate9Ex);
            //if every thing is ok. mark this module detoured.
            if(!DetourTransactionCommit())
                MarkModuleDetoured(module);
        }
        locker_.Release();
    }  
}

}
}