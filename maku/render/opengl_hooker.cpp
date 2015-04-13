
#include "opengl_hooker.h"
#include <ncore/sys/spin_lock.h>
#include "opengl_common.h"
#include "detours\detours.h"
#include "detours\detours_ext.h"
#include "opengl_canvas.h"
#include "render_context.h"
#include "input_hooker.h"
#include "utils.h"

namespace maku
{
namespace render
{
namespace opengl
{

typedef LazyMap<HDC, OpenGLCanvas *> DC2Canvas;

ncore::SpinLock locker;
DC2Canvas dc2canvas;

SwapBuffers_T TSwapBuffers = NULL;   
wglDeleteContext_T TwglDeleteContext = NULL;
wglMakeCurrent_T TwglMakeCurrent = NULL;

void OnPresenting(HDC dc);

void InsertRC(HDC dc, HGLRC hrc);

void DeleteRC(HGLRC hrc);

OpenGLCanvas * GetCanvasByDC(HDC dc);
//////////////////////////////////////////////////////////////////////////

/*********************************************
 FSwapBuffers
 调用自己的绘制例程
 **********************************************/
BOOL WINAPI FSwapBuffers(HDC dc)
{
    OnPresenting(dc);
    return TSwapBuffers(dc);
}

/*********************************************
 FwglDeleteContext
 HGLRC删除后 删除对应的Canvas
 **********************************************/
BOOL  WINAPI FwglDeleteContext(HGLRC hrc)
{
    if (hrc)
        DeleteRC(hrc);

    return TwglDeleteContext(hrc);
}

/*********************************************
 FwglMakeCurrent
 更新DC对应的Canvas
 **********************************************/
BOOL  WINAPI FwglMakeCurrent(HDC dc, HGLRC hrc)
{
    BOOL ret = TwglMakeCurrent(dc, hrc);
    if (dc && hrc)
        InsertRC(dc, hrc);  

    return ret;
}

void OnPresenting(HDC dc)
{
    HWND hwnd = ::WindowFromDC(dc);
    HWND cur_hwnd = InputHooker::GetCurrentHWND();
    if (hwnd != cur_hwnd && !IsChild(cur_hwnd, hwnd))
        return;

    RECT rect;
    ::GetClientRect(hwnd, &rect);

    RenderContext::Get()->UpdateStatus(rect.right - rect.left,
        rect.bottom - rect.top);

    if (RenderContext::Get()->IsPresent())
    {
        //HOOK ChangeDisplaySettings ChangeDisplaySettingsEx获取是否全屏模式
        //暂未实现
        InputHooker::SetTransRect(rect, rect);

        OpenGLCanvas * opengl_canvas = GetCanvasByDC(dc);
        if (opengl_canvas)
        {
            opengl_canvas->UpdateRect(rect);
            opengl_canvas->SaveStatus();
            Canvas* canvas = opengl_canvas;
            RenderContext::Get()->Draw(canvas);
            opengl_canvas->RestoreStatus();
        }
    }
}

void InsertRC(HDC dc, HGLRC hrc)
{
    HWND hwnd = ::WindowFromDC(dc);
    locker.Acquire();
    //if exist, delete
    if(dc2canvas.Has(dc))
        delete dc2canvas[dc];

    //insert new canvas
    OpenGLCanvas * opengl_canvas = new OpenGLCanvas(hrc);
    dc2canvas[dc] = opengl_canvas;
    //release
    locker.Release();
}

void DeleteRC(HGLRC hrc)
{
    locker.Acquire();

    DC2Canvas::PairArray array = dc2canvas.Items();

    for(auto iter = array.begin(); iter != array.end(); ++iter)
    {
        HDC hdc = iter->first;
        OpenGLCanvas * canvas = iter->second;
        if(canvas->hrc() == hrc)
        {
            delete canvas;
            dc2canvas.Remove(hdc);
        }
    }

    locker.Release();
}

OpenGLCanvas * GetCanvasByDC(HDC dc)
{
    OpenGLCanvas * canvas = NULL;

    locker.Acquire();    
    if(dc2canvas.Has(dc))
        canvas = dc2canvas[dc];
    locker.Release();

    return canvas;
}

       
}
//////////////////////////////////////////////////////////////////////////
ncore::SpinLock OpenGLHooker::locker_;

void OpenGLHooker::Hook()
{
    using namespace opengl;
    wchar_t opengl32_path[MAX_PATH] =  {0};
    if( 0 == ::GetSystemDirectory(opengl32_path, sizeof(opengl32_path)) )
        return;
    wcscat_s(opengl32_path, L"\\opengl32.dll");
    void* gl_module = ::GetModuleHandle(opengl32_path);

    wchar_t gdi32_path[MAX_PATH] =  {0};
    if( 0 == ::GetSystemDirectory(gdi32_path, sizeof(gdi32_path)) )
        return;
    wcscat_s(gdi32_path, L"\\gdi32.dll");
    void* gdi_module = ::GetModuleHandle(gdi32_path);    

    if(gl_module == NULL || gdi_module == NULL)
        return;

    if (!IsModuleDetoured(gl_module) && !IsModuleDetoured(gdi_module))
    {
        if (!locker_.TryAcquire())
            return;
        if (!IsModuleDetoured(gl_module) && !IsModuleDetoured(gdi_module))
        {
            HMODULE gl_mod = (HMODULE)gl_module;
            HMODULE gdi_mod = (HMODULE)gdi_module;

            TSwapBuffers = (SwapBuffers_T)GetProcAddress(gdi_mod, "SwapBuffers");
            TwglMakeCurrent = (wglMakeCurrent_T)GetProcAddress(gl_mod, "wglMakeCurrent");
            TwglDeleteContext = (wglDeleteContext_T)GetProcAddress(gl_mod, "wglDeleteContext");

            DetourTransactionBegin();
            DetourUpdateThread(::GetCurrentThread());

            if (TSwapBuffers)
                DetourAttach(&(PVOID&)TSwapBuffers, FSwapBuffers);
            if(TwglDeleteContext)
                DetourAttach(&(PVOID&)TwglDeleteContext, FwglDeleteContext);
            if(TwglMakeCurrent)
                DetourAttach(&(PVOID&)TwglMakeCurrent, FwglMakeCurrent);

            if(!DetourTransactionCommit())
            {
                MarkModuleDetoured(gl_module);
                MarkModuleDetoured(gdi_module);
            }
        }
        locker_.Release();
    }
}    
    }
}
