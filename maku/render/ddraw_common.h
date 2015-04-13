#ifndef MAKU_RENDER_DDRAW_COMMON_H_
#define MAKU_RENDER_DDRAW_COMMON_H_

//ddraw headers will refer to windows header.
//without this macro that will lead to confilcit with winsock2 header
//we deinfe it outself.
#include <ncore/ncore.h>
#include <ddraw.h>
#include "d3d7/d3d7.h"

namespace maku
{
namespace render
{
namespace ddrawhooker
{
const GUID IID_IDirectDraw_T  = {0x6C14DB80,0xA733,
    0x11CE,0xA5,0x21,0x00,0x20,0xAF,0x0B,0xE5,0x60};
const GUID IID_IDirectDraw7_T = {0x15e65ec0,0x3b9c,
    0x11d2,0xb9,0x2f,0x00,0x60,0x97,0x97,0xea,0x5b};
const GUID IID_IDirect3D7_T   = {0xf5049e77,0x4861,
    0x11d2,0xa4,0x7,0x0,0xa0,0xc9,0x6,0x29,0xa8};
const GUID IID_IDirectDrawSurface7_T =    {0x06675a80,0x3b9b,
    0x11d2,0xb9,0x2f,0x00,0x60,0x97,0x97,0xea,0x5b};
//IUnknown
static const size_t kIdxQueryInterface = 0x00;

//IDirectDraw
static const size_t kIdxSetCooperativeLevel  = 0x14;
static const size_t kIdxCreateSurface = 6;

typedef HRESULT (WINAPI * DirectDrawCreate_T)(
    GUID FAR *lpGUID,
    LPDIRECTDRAW FAR *lplpDD,
    IUnknown FAR *pUnkOuter);

typedef HRESULT (WINAPI * DirectDrawCreateEx_T)(
    GUID FAR *lpGUID,
    LPDIRECTDRAW FAR *lplpDD,
    REFIID iid,
    IUnknown FAR *pUnkOuter);

typedef HRESULT (WINAPI * QueryInterface_T)(
    IDirectDraw7 *self,
    REFIID riid, 
    LPVOID * ppvObj);

typedef HRESULT (WINAPI * SetCooperativeLevel_T)(
    IDirectDraw *self,
    HWND hWnd,
    DWORD dwFlags);

typedef HRESULT (WINAPI * DDraw7CreateSurface_T)(
    IDirectDraw7 *self,
    LPDDSURFACEDESC2, 
    LPDIRECTDRAWSURFACE7 FAR *, 
    IUnknown FAR *);
//IDirectDrawSurface7
static size_t kIdxFlip = 11;
typedef HRESULT (WINAPI * DDrawSurface7Flip_T)(
    IDirectDrawSurface7*,LPDIRECTDRAWSURFACE7, DWORD);

static size_t kIdxBltFast = 7;
typedef HRESULT (WINAPI * DDrawSurface7BltFast_T)(
    IDirectDrawSurface7*, 
    DWORD,DWORD,LPDIRECTDRAWSURFACE7, LPRECT,DWORD);

static size_t kIdxBlt = 5;
typedef HRESULT (WINAPI * DDrawSurface7Blt_T)(
    IDirectDrawSurface7 *,
    LPRECT lpDestRect,
    LPDIRECTDRAWSURFACE7 lpDDSrcSurface,
    LPRECT lpSrcRect,
    DWORD dwFlags,
    LPDDBLTFX lpDDBltFx
    );
//IDirect3D7
static const size_t kIdxD3D7CreateDevice              = 0x04;
//IDirect3DDevice7
static const size_t kIdxD3D7Release                       = 0x02;
static const size_t kIdxD3D7EndScene                  = 0x06;
static const size_t kIdxD3D7Clear                     = 0x0A;


typedef HRESULT (WINAPI * D3D7CreateDevice_T)(
    IDirect3D7 *self,
    THIS_ REFCLSID rclsid,
    LPDIRECTDRAWSURFACE7 lpDDS, 
    LPDIRECT3DDEVICE7* lplpD3DDevice);

typedef HRESULT (WINAPI* DDraw7Release_T)(
    IDirectDraw7* self);

typedef HRESULT(WINAPI * D3D7Release_T)(
    IDirect3DDevice7 *self);


}
}
}
#endif