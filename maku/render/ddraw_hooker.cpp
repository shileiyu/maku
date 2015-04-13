//ddraw headers will refer to windows header.
//without this macro that will lead to conflict with winsock2 header
//we define it ourself.

#include "ddraw_hooker.h"
#include <ncore\sys\spin_lock.h>
#include "detours\detours.h"
#include "detours\detours_ext.h"
#include "ddraw_common.h"
#include "d3d7_canvas.h"
#include "utils.h"
#include "render_context.h"
#include "input_hooker.h"
#include "gdi_canvas.h"

namespace maku
{
namespace render
{
namespace ddrawhooker
{
//guid declare(to avoid include "ddraw.lib")
//directdraw--hwnd 
typedef LazyMap<IDirectDraw7*, HWND> Draw72HWND;
typedef LazyMap<IDirect3DDevice7*, D3D7Canvas*> Device72Canvas;

Draw72HWND ddraw_hwnd;
Device72Canvas device7_canvas;

ncore::SpinLock locker_hwnd;
ncore::SpinLock locker_canvas;

GDICanvas gdi_canvas;

//original function address
DirectDrawCreate_T TDirectDrawCreate = 0;
DirectDrawCreateEx_T TDirectDrawCreateEx = 0;
QueryInterface_T TQueryInterface = 0;
D3D7CreateDevice_T TD3D7CreateDevice = 0;
SetCooperativeLevel_T TSetCooperativeLevel= 0;
D3D7Release_T TD3D7Release = 0;
DDraw7Release_T TDDraw7Release = 0;
DDrawSurface7Flip_T TDDrawSurface7Flip = 0;
DDraw7CreateSurface_T TDDraw7CreateSurface = 0;
DDrawSurface7BltFast_T TDDrawSurface7BltFast = 0; 
DDrawSurface7Blt_T TDDrawSurface7Blt = 0;

//detour function declare
HRESULT WINAPI FQueryInterface(IDirectDraw7 * self,
                               REFIID riid, 
                               LPVOID * ppvObj);

HRESULT WINAPI FSetCooperativeLevel(IDirectDraw * self,
                                    HWND hWnd,
                                    DWORD dwFlags);

HRESULT WINAPI FD3D7CreateDevice(IDirect3D7 * self,
                                 THIS_ REFCLSID rclsid,
                                 LPDIRECTDRAWSURFACE7 lpDDS, 
                                 LPDIRECT3DDEVICE7* lplpD3DDevice);

ULONG WINAPI FD3D7Release(IDirect3DDevice7 * self);

ULONG WINAPI FDDraw7Release(IDirectDraw7 * self);

HRESULT WINAPI FDDrawSurface7Flip(
    IDirectDrawSurface7* self, 
    LPDIRECTDRAWSURFACE7 target, 
    DWORD flag);

HRESULT WINAPI FDDraw7CreateSurface(
    IDirectDraw7 *self,
    LPDDSURFACEDESC2 desc,  
    LPDIRECTDRAWSURFACE7 FAR * out, 
    IUnknown FAR * unknown);

HRESULT WINAPI FDDrawSurface7BltFast(
    IDirectDrawSurface7*, 
    DWORD,DWORD,LPDIRECTDRAWSURFACE7, LPRECT,DWORD);

HRESULT WINAPI FDDrawSurface7Blt(
    IDirectDrawSurface7 * self,
    LPRECT lpDestRect,
    LPDIRECTDRAWSURFACE7 lpDDSrcSurface,
    LPRECT lpSrcRect,
    DWORD dwFlags,
    LPDDBLTFX lpDDBltFx
    );

void DetourD3D7Release(IDirect3DDevice7 * i7e);

void DetourDDraw7Release(IDirectDraw7 * i7e);

void DetourCreateDevice(IDirect3D7 * i7e);

void DetourSetCooperativeLevel(IDirectDraw7 * i7e);

void DetourQueryInterface(IDirectDraw7 * i7e);

bool SetGameParm(HWND hwnd, uint32_t width, uint32_t height);

void InsertDevice(IDirect3DDevice7 * device);

void DeleteDevice(IDirect3DDevice7 * device);

D3D7Canvas* GetCanvasByDevice(IDirect3DDevice7 * device);

HWND FindHwndByDDraw7(IDirectDraw7 * draw7);

D3D7Canvas* GetCanvasByDDraw7(IDirectDraw7 * ddraw7);

//Implement
void DetourQueryInterface(IDirectDraw7 * i7e)
{
    if(IsVFDetoured(i7e, kIdxQueryInterface))
        return;

    DWORD temp = 0;
    GetVFAddress(i7e, kIdxD3D7Release, temp);
    //Detach hook before Attach it again.
    if(GetVFAddress(i7e, kIdxQueryInterface, TQueryInterface))
        DetourAttach((void**)&TQueryInterface, FQueryInterface);
}

void DetourCreateDevice(IDirect3D7 * i7e)
{
    if(IsVFDetoured(i7e, kIdxD3D7CreateDevice))
        return;

    //Detach hook before Attach it again.
    if(GetVFAddress(i7e, kIdxD3D7CreateDevice, TD3D7CreateDevice))
        DetourAttach((void**)&TD3D7CreateDevice, FD3D7CreateDevice);
}

void DetourSetCooperativeLevel(IDirectDraw7 * i7e)
{
    if(IsVFDetoured(i7e, kIdxSetCooperativeLevel))
        return;

    //Detach hook before Attach it again.
    if(GetVFAddress(i7e, kIdxSetCooperativeLevel, TSetCooperativeLevel))
        DetourAttach((void**)&TSetCooperativeLevel, FSetCooperativeLevel);
}

void DetourD3D7Release(IDirect3DDevice7 * i7e)
{
    if(IsVFDetoured(i7e, kIdxD3D7Release))
        return;

    if(GetVFAddress(i7e, kIdxD3D7Release, TD3D7Release))
        DetourAttach((void **)&TD3D7Release, FD3D7Release);
}

void DetourDDraw7Release(IDirectDraw7 * i7e)
{
    if(IsVFDetoured(i7e, kIdxD3D7Release))
        return;

    if(GetVFAddress(i7e, kIdxD3D7Release, TDDraw7Release))
        DetourAttach((void **)&TDDraw7Release, FDDraw7Release);
}


void DetourDDraw7CreateSurface(IDirectDraw7 * i7e)
{
    if(IsVFDetoured(i7e, kIdxCreateSurface))
        return;

    if(GetVFAddress(i7e, kIdxCreateSurface, TDDraw7CreateSurface))
        DetourAttach((void **)&TDDraw7CreateSurface, FDDraw7CreateSurface);
}

void DetourDDrawSurface7Flip(IDirectDrawSurface7 * i7e)
{
    if(IsVFDetoured(i7e, kIdxFlip))
        return;

    if(GetVFAddress(i7e, kIdxFlip, TDDrawSurface7Flip))
        DetourAttach((void **)&TDDrawSurface7Flip, FDDrawSurface7Flip);
}

void DetourDDrawSurface7BltFast(IDirectDrawSurface7 * i7e)
{
    if(IsVFDetoured(i7e, kIdxBltFast))
        return;

    if(GetVFAddress(i7e, kIdxBltFast, TDDrawSurface7BltFast))
        DetourAttach((void **)&TDDrawSurface7BltFast, FDDrawSurface7BltFast);

}

void DetourDDrawSurface7Blt(IDirectDrawSurface7 * i7e)
{
    if(IsVFDetoured(i7e, kIdxBlt))
        return;

    if(GetVFAddress(i7e, kIdxBlt, TDDrawSurface7Blt))
        DetourAttach((void **)&TDDrawSurface7Blt, FDDrawSurface7Blt);

}

HRESULT WINAPI FDDraw7CreateSurface(
    IDirectDraw7 *self,
    LPDDSURFACEDESC2 desc,  
    LPDIRECTDRAWSURFACE7 FAR * out, 
    IUnknown FAR * unknown)
{

    auto hr = TDDraw7CreateSurface(self, desc, out, unknown);
    if(SUCCEEDED(hr) && ( desc->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE ))
    {
        DetourTransactionBegin();
        DetourUpdateThread(::GetCurrentThread());
        DetourDDrawSurface7Flip(*out);
        DetourDDrawSurface7Blt(*out);
        DetourDDrawSurface7BltFast(*out);
        DetourTransactionCommit();
    }
    return hr;
}

void OnPresenting(LPDIRECTDRAWSURFACE7 target, uint32_t width, uint32_t height, const Rect & rect)
{
    

    if(target == 0 || rect.isEmpty())
        return;

    IUnknown * unknown = 0;
    target->GetDDInterface((void **)&unknown);
    IDirectDraw7 * ddraw7 = 0;
    unknown->QueryInterface(IID_IDirectDraw7_T, (void **)&ddraw7);
    unknown->Release();
    

    //检查句柄
    HWND game_hwnd = FindHwndByDDraw7(ddraw7);
    HWND cur_hwnd = InputHooker::GetCurrentHWND();

    if(game_hwnd != cur_hwnd && !IsChild(cur_hwnd, game_hwnd))
        return;

    RenderContext::Get()->UpdateStatus(rect.width(), rect.height());
    if (RenderContext::Get()->IsPresent())
    {
        if (!SetGameParm(game_hwnd, rect.width(), rect.height()))
            return;
        
        D3D7Canvas * d3d7_canvas = GetCanvasByDDraw7(ddraw7);
        if( d3d7_canvas)
        {
            d3d7_canvas->SetRenderTarget(target, rect);
            d3d7_canvas->SaveStatus();
            RenderContext::Get()->Draw(d3d7_canvas);
            d3d7_canvas->RestoreStatus();
        }
        else
        {
            HDC dc = 0;
            if(SUCCEEDED(target->GetDC(&dc)))
            {
                gdi_canvas.SetRenderTarget(dc, width, height, rect);
                gdi_canvas.SaveStatus();
                RenderContext::Get()->Draw(&gdi_canvas);
                gdi_canvas.RestoreStatus();
                target->ReleaseDC(dc);
            }
        }
    }
    ddraw7->Release();

    
    ;
}

bool IsPrimarySurface(IDirectDrawSurface7 * surface)
{
    DDSURFACEDESC2 desc2;
    desc2.dwSize = sizeof(desc2);
    HRESULT hr = surface->GetSurfaceDesc(&desc2);

    if(SUCCEEDED(hr))
    {
        if(desc2.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
            return true;

        return false;
    }

    //may be IDirectDrawSurface
    DDSURFACEDESC desc;
    desc.dwSize = sizeof(desc);
    hr  = surface->GetSurfaceDesc( (DDSURFACEDESC2 *) &desc);
    if(SUCCEEDED(hr))
    {
        if(desc.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
            return true;
    }

    return false;
}

HRESULT WINAPI FDDrawSurface7Flip(
    IDirectDrawSurface7* self, LPDIRECTDRAWSURFACE7 target, DWORD flag)
{
    if(IsPrimarySurface(self) )
    {//self是主平面
        LPDIRECTDRAWSURFACE7 render_target = 0;
        if(0 == render_target)
        {
            //取后备缓冲区
            LPDIRECTDRAWSURFACE7 back_buffer = 0;
            DDSCAPS2 caps2 = {0};
            caps2.dwCaps = DDSCAPS_BACKBUFFER;
            if(FAILED(self->GetAttachedSurface(&caps2, &back_buffer)))
                return TDDrawSurface7Flip(self, target, flag);

            back_buffer->QueryInterface(IID_IDirectDrawSurface7_T, (void **)&render_target);
            back_buffer->Release();
        }
        else
        {
            target->QueryInterface(IID_IDirectDrawSurface7_T, (void **)&render_target);
        }
        if(render_target)
        {
            DDSURFACEDESC2 desc;
            desc.dwSize = sizeof(desc);
            render_target->GetSurfaceDesc(&desc);
            Rect rect;
            rect.SetXYWH(0, 0, desc.dwWidth, desc.dwHeight);
            OnPresenting(render_target, desc.dwWidth, desc.dwHeight, rect);
            render_target->Release();
        }

    }
    return TDDrawSurface7Flip(self, target, flag);
}

HRESULT WINAPI FDDrawSurface7Blt(
    IDirectDrawSurface7 * self,
    LPRECT lpDestRect,
    LPDIRECTDRAWSURFACE7 lpDDSrcSurface,
    LPRECT lpSrcRect,
    DWORD dwFlags,
    LPDDBLTFX lpDDBltFx
    )
{
    if(IsPrimarySurface(self) && !(dwFlags & DDBLT_ASYNC) )
    {//主平面
        if(lpDDSrcSurface)
        {
            LPDIRECTDRAWSURFACE7 lpDDSrcSurface7 = 0;
            lpDDSrcSurface->QueryInterface(IID_IDirectDrawSurface7_T, 
                (void **)&lpDDSrcSurface7);

            if(lpDDSrcSurface7 == 0)
                return TDDrawSurface7Blt(self, lpDestRect, lpDDSrcSurface, 
                lpSrcRect, dwFlags, lpDDBltFx);

            DDSURFACEDESC2 desc;
            desc.dwSize = sizeof(desc);
            lpDDSrcSurface7->GetSurfaceDesc(&desc);
            Rect rect;
            if(lpSrcRect)
                rect.SetLTRB(lpSrcRect->left, lpSrcRect->top, lpSrcRect->right, lpSrcRect->bottom);
            else
                rect.SetXYWH(0, 0, desc.dwWidth, desc.dwHeight);

            OnPresenting(lpDDSrcSurface7, desc.dwWidth, desc.dwHeight, rect);

            //平衡引用计数
            lpDDSrcSurface7->Release();
        }
    }
    return TDDrawSurface7Blt(self, lpDestRect, lpDDSrcSurface, lpSrcRect, dwFlags, lpDDBltFx);
}

HRESULT WINAPI FDDrawSurface7BltFast(
    IDirectDrawSurface7 * self, 
    DWORD dwX,DWORD dwY, LPDIRECTDRAWSURFACE7 lpDDSrcSurface, 
    LPRECT lpSrcRect, DWORD dwFlags)
{
    if(IsPrimarySurface(self))
    {//主平面
        if(lpDDSrcSurface)
        {
            LPDIRECTDRAWSURFACE7 lpDDSrcSurface7 = 0;
            lpDDSrcSurface->QueryInterface(IID_IDirectDrawSurface7_T, 
                (void **)&lpDDSrcSurface7);

            if(lpDDSrcSurface7 == 0)
                return TDDrawSurface7BltFast(self, dwX, dwY, lpDDSrcSurface, 
                                             lpSrcRect, dwFlags);

            DDSURFACEDESC2 desc;
            desc.dwSize = sizeof(desc);
            lpDDSrcSurface7->GetSurfaceDesc(&desc);
            Rect rect;
            if(lpSrcRect)
                rect.SetLTRB(lpSrcRect->left, lpSrcRect->top, lpSrcRect->right, lpSrcRect->bottom);
            else
                rect.SetXYWH(0, 0, desc.dwWidth, desc.dwHeight);

            OnPresenting(lpDDSrcSurface7, desc.dwWidth, desc.dwHeight, rect);
            //平衡引用计数
            lpDDSrcSurface7->Release();
        }
    }
    return TDDrawSurface7BltFast(self, dwX, dwY, lpDDSrcSurface, lpSrcRect, dwFlags);
}

/*********************************************
 FDirectDrawCreate
 暂时无用
 **********************************************/
HRESULT WINAPI FDirectDrawCreate(GUID FAR * lpGUID,
                                 LPDIRECTDRAW FAR *lplpDD,
                                 IUnknown FAR *pUnkOuter)
{
    HRESULT hr;
    hr = TDirectDrawCreate(lpGUID,lplpDD,pUnkOuter);
    return hr;
}

/*********************************************
 FDirectDrawCreateEx

 HOOK IDirectDraw7::QueryInterface
      IDirectDraw7::Release
 **********************************************/
HRESULT WINAPI FDirectDrawCreateEx(GUID FAR *lpGUID,
                                   LPDIRECTDRAW FAR *lplpDD,
                                   REFIID iid,
                                   IUnknown FAR *pUnkOuter)
{
    HRESULT hr;
    hr = TDirectDrawCreateEx(lpGUID,lplpDD,iid,pUnkOuter);
    if (SUCCEEDED(hr) && lplpDD != NULL)
    {
        if(iid == IID_IDirectDraw7_T)
        {
            auto i7e = *((IDirectDraw7**)lplpDD);

            DetourTransactionBegin();
            DetourUpdateThread(::GetCurrentThread());
            DetourQueryInterface(i7e);    
            DetourDDraw7Release(i7e);
            DetourSetCooperativeLevel(i7e);
            DetourDDraw7CreateSurface(i7e);
            DetourTransactionCommit();
        }
    }

    return hr;
}

/*********************************************
 FQueryInterface

 HOOK IDirect3D7::CreateDevice
      IDirectDraw::SetCooperativeLevel
 **********************************************/
HRESULT WINAPI FQueryInterface(IDirectDraw7 * self,
                               REFIID riid, 
                               LPVOID * ppvObj)
{
    if(TQueryInterface == 0)
        return S_FALSE;
    
    HRESULT hr;
    hr = TQueryInterface(self, riid, ppvObj);
    if (SUCCEEDED(hr) && ppvObj != NULL && *(DWORD*)ppvObj != NULL)
    {
        //hook d3d7 vft
        if(riid == IID_IDirect3D7_T)
        {
            IDirect3D7* d3d7_ptr = (IDirect3D7*)(*ppvObj);

            DetourTransactionBegin();
            DetourUpdateThread(::GetCurrentThread());
            DetourCreateDevice(d3d7_ptr);
            DetourTransactionCommit();
        }
    }
    return hr;
}

/*********************************************
 FSetCooperativeLevel

 HOOK IDirectDraw::SetCooperativeLevel
      用于获取IDirectDraw对应的窗口句柄
      句柄保存在全局变量ddraw_hwnd中
 **********************************************/
HRESULT WINAPI FSetCooperativeLevel(IDirectDraw * self,
                                    HWND hWnd,
                                    DWORD dwFlags)
{
    if(TSetCooperativeLevel)
    {
        HRESULT hr = TSetCooperativeLevel(self, hWnd, dwFlags);
        if (SUCCEEDED(hr))
        {
            //get ddraw7
            IDirectDraw7* ddraw = NULL;
            self->QueryInterface(IID_IDirectDraw7_T, (void**)&ddraw);
            ddraw->Release();
            if (ddraw)
            {
                locker_hwnd.Acquire();
                ddraw_hwnd[ddraw] = hWnd;
                locker_hwnd.Release();
            }
        }
        return hr;
    }
    else
        return S_FALSE;
}

// DirectD3D7 fake functions
/*********************************************
 FD3D7CreateDevice

 HOOK IDirect3DDevice7::EndScene
      IDirect3DDevice7::Clear
      IDirect3DDevice7::Release
 缓存IDirect3DDevice7指针到device7_canvas中
 **********************************************/
HRESULT WINAPI FD3D7CreateDevice(IDirect3D7 * self,
                                 THIS_ REFCLSID rclsid,
                                 LPDIRECTDRAWSURFACE7 lpDDS, 
                                 LPDIRECT3DDEVICE7* lplpD3DDevice)
{
    if(TD3D7CreateDevice)
    {
        IDirect3DDevice7 * i7e = 0;
        auto hr = TD3D7CreateDevice(self, rclsid, lpDDS, lplpD3DDevice);
        if(lplpD3DDevice)
            i7e = *lplpD3DDevice;
        if(hr == S_OK && i7e)
        {            
            DetourTransactionBegin();
            DetourUpdateThread(::GetCurrentThread());
            InsertDevice(i7e);
            //hook
            DetourD3D7Release(i7e);

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
 FDDraw7Release
 当DDraw释放时将ddraw_hwnd缓存的窗口句柄释放
 **********************************************/
ULONG WINAPI FDDraw7Release(IDirectDraw7 * self)
{
    ULONG ret = 0;
    if (TDDraw7Release)
    {
        ret = TDDraw7Release(self);
        if (ret == 0)
        {
            locker_hwnd.Acquire();
            if(ddraw_hwnd.Has(self))
                ddraw_hwnd.Remove(self);
            locker_hwnd.Release();
        }
    }
    return ret;
}
/*********************************************
 FD3D7Release
 当IDirect3DDevice7释放时将device7_canvas缓存的
 D3D7Canvas释放
 **********************************************/
ULONG WINAPI FD3D7Release(IDirect3DDevice7 * self)
{
    ULONG ret = 0;
    if(TD3D7Release)
    {
        ret = TD3D7Release(self);    
        if (ret == 0)
        {
            D3D7Canvas* d3d7_canvas = GetCanvasByDevice(self);
            if (d3d7_canvas)
            {
                DeleteDevice(self);
                ret = 0;
            }
        }  
    }
    return ret;
}

void InsertDevice(IDirect3DDevice7 * device)
{
    locker_canvas.Acquire();
    D3D7Canvas * d3d7_canvas = new D3D7Canvas(device);
    device7_canvas[device] = d3d7_canvas;
    locker_canvas.Release();
}

void DeleteDevice(IDirect3DDevice7 * device)
{
    D3D7Canvas * d3d7_canvas = NULL;
    locker_canvas.Acquire();
    if(device7_canvas.Has(device))
    {
        d3d7_canvas = device7_canvas[device];
        device7_canvas.Remove(device);
    }
    locker_canvas.Release();
    if(d3d7_canvas)
        delete d3d7_canvas;
}

D3D7Canvas* GetCanvasByDevice(IDirect3DDevice7 * device)
{
    D3D7Canvas * temp = NULL;

    locker_canvas.Acquire();
    if(device7_canvas.Has(device))
        temp = device7_canvas[device];
    locker_canvas.Release();

    return temp;
}

D3D7Canvas* GetCanvasByDDraw7(IDirectDraw7 * ddraw7)
{
    D3D7Canvas * temp = NULL;

    locker_canvas.Acquire();
    auto items = device7_canvas.Items();
    for(auto iter = items.begin(); iter != items.end(); ++iter)
    {
        IDirect3D7* dd7 = NULL;
        IDirectDraw7* ddraw = NULL;
        iter->first->GetDirect3D(&dd7);
        if(!dd7)
            continue;
        dd7->QueryInterface(IID_IDirectDraw7_T, (void**)&ddraw);
        dd7->Release();
        if(!ddraw)
            continue;
        ddraw->Release();
        if(ddraw == ddraw7)
        {
            temp = iter->second;
            break;
        }
    }
    locker_canvas.Release();

    return temp;
}

//find device<-->hwnd
HWND FindHwndByDDraw7(IDirectDraw7 * draw7)
{
    HWND hwnd = NULL;
    locker_hwnd.Acquire();
    if(ddraw_hwnd.Has(draw7))
        hwnd = ddraw_hwnd[draw7];
    locker_hwnd.Release();

    return hwnd;
}

HWND GetDeviceHWND(IDirect3DDevice7 * device)
{
    IDirect3D7* dd7 = NULL;
    IDirectDraw7* ddraw = NULL;
    HWND hwnd = NULL;

    device->GetDirect3D(&dd7);
    dd7->QueryInterface(IID_IDirectDraw7_T, (void**)&ddraw);

    if (ddraw != NULL)
    {
        dd7->Release();
        ddraw->Release();
    }

    locker_hwnd.Acquire();
    if(ddraw_hwnd.Has(ddraw))
        hwnd = ddraw_hwnd[ddraw];
    locker_hwnd.Release();

    return hwnd;
}

/*********************************************
 SetPresentParm
 将绘制时获得的参数传递给InputHooker 以便进行
 消息转换
 **********************************************/
bool SetGameParm(HWND hwnd, uint32_t width, uint32_t height)
{
    //
    RECT w_rect;
    GetClientRect(hwnd, &w_rect);

    RECT mix_rect;
    mix_rect.left = mix_rect.top = 0;
    mix_rect.right = width;
    mix_rect.bottom = height;

    InputHooker::SetTransRect(w_rect, mix_rect);
    return true;
}


}

//////////////////////////////////////////////////////////////////////////
ncore::SpinLock DDrawHooker::locker_;

/*********************************************
 DDrawHooker::Hook
 Hook主函数由Kernel32Hooker 以及DllMain调用
 Hook DirectDrawCreate
      DirectDrawCreateEx
 **********************************************/
void DDrawHooker::Hook()
{
    using namespace ddrawhooker;
    wchar_t ddraw_path[MAX_PATH] =  {0};
    if( 0 == ::GetSystemDirectory(ddraw_path, sizeof(ddraw_path)) )
        return;
    wcscat_s(ddraw_path, L"\\ddraw.dll");

    void * module = ::GetModuleHandle(ddraw_path);
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

            proc = GetProcAddress(mod, "DirectDrawCreate");
            TDirectDrawCreate  = (DirectDrawCreate_T)proc;

            proc = GetProcAddress(mod, "DirectDrawCreateEx");
            TDirectDrawCreateEx  = (DirectDrawCreateEx_T)proc;

            HANDLE curr_thread = GetCurrentThread();
            DetourTransactionBegin();
            DetourUpdateThread(curr_thread);
            if(TDirectDrawCreate)
                DetourAttach(&(PVOID&)TDirectDrawCreate, FDirectDrawCreate);
            if(TDirectDrawCreateEx)
                DetourAttach(&(PVOID&)TDirectDrawCreateEx, FDirectDrawCreateEx);

            //if every thing is ok. mark this module detoured.
            if(!DetourTransactionCommit())
                MarkModuleDetoured(module);
        }
        locker_.Release();
    } 
}

}
}