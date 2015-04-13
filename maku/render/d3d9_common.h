#ifndef MAKU_RENDER_D3D9_COMMON_H_
#define MAKU_RENDER_D3D9_COMMON_H_

//d3d9 headers will refer to windows header.
//without this macro that will lead to confilcit with winsock2 header
//we deinfe it outself.
#include <ncore/ncore.h>
#include <d3d9.h>

namespace maku
{
namespace render
{
namespace d3d9hooker
{
 //IUnknown
 static const size_t kIdxRelease                          = 0x02;
//IDirect3D9
static const size_t kIdxCreateDevice                     = 0x10;
//IDirect3D9Ex
static const size_t kIdxCreateDeviceEx                     = 0x14;
static const size_t kIdxResetEx                            = 0x84;
static const size_t kIdxPresentEx                          = 0x79;
//IDirect3DDevice9
static const size_t kIdxCreateAdditionalSwapChain        = 0x0D;
static const size_t kIdxGetSwapChain                     = 0x0E;
static const size_t kIdxReset                            = 0x10;
static const size_t kIdxPresent                          = 0x11;
static const size_t kIdxSetRenderState                   = 0x39;
static const size_t kIdxSetSamplerState                  = 0x45;
static const size_t kIdxSetTextureStageState             = 0x43;
static const size_t kIdxSetPixelShader                   = 0x6B;
//IDirect3DSwapChian9
static const size_t kIdxSwapChainPresent                 = 0x03;

typedef IDirect3D9 * (WINAPI * Direct3DCreate9_T)(UINT SDKVersion);

typedef HRESULT (WINAPI * Direct3DCreate9Ex_T)(UINT SDKVersion, 
                                               IDirect3D9Ex ** ptr);

typedef HRESULT (WINAPI * CreateDevice_T)(IDirect3D9 * self, 
                                          UINT Adapter,
                                          D3DDEVTYPE DeviceType,
                                          HWND hFocusWindow,
                                          DWORD BehaviorFlags,
                                          D3DPRESENT_PARAMETERS * pParams,
                                          IDirect3DDevice9 ** ppDeviceIntf);

typedef HRESULT (WINAPI * CreateDeviceEx_T)(
    IDirect3D9Ex * self,
    UINT Adapter,
    D3DDEVTYPE DeviceType,
    HWND hFocusWindow,
    DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS* pPresentationParameters,
    D3DDISPLAYMODEEX* pFullscreenDisplayMode,
    IDirect3DDevice9Ex** ppReturnedDeviceInterface
    );

typedef HRESULT (WINAPI * GetSwapChain_T)(IDirect3DDevice9 * self, 
                                          UINT iSwapChain,
                                          IDirect3DSwapChain9 ** pSwapChain);

typedef HRESULT (WINAPI * CreateAdditionalSwapChain_T)(IDirect3DDevice9 * self, 
                                                       D3DPRESENT_PARAMETERS * pPresentationParameters,
                                                       IDirect3DSwapChain9 ** pSwapChain);

typedef ULONG (WINAPI * Release_T)(IUnknown * self);

typedef HRESULT (WINAPI * Reset_T)(IDirect3DDevice9 * self, 
    D3DPRESENT_PARAMETERS * pPresentationParameters);

typedef HRESULT (WINAPI * ResetEx_T)(
    IDirect3DDevice9Ex * self,
    D3DPRESENT_PARAMETERS *pPresentationParameters,
    D3DDISPLAYMODEEX *pFullscreenDisplayMode
    );

typedef HRESULT (WINAPI * Present_T)(IDirect3DDevice9 * self, 
                                     CONST RECT * pSourceRect,
                                     CONST RECT * pDestRect,
                                     HWND hDestWindowOverride,
                                     CONST RGNDATA * pDirtyRegion);

typedef HRESULT (WINAPI * PresentEx_T)(
    IDirect3DDevice9Ex * self,
    const RECT *pSourceRect,
    const RECT *pDestRect,
    HWND hDestWindowOverride,
    const RGNDATA *pDirtyRegion,
    DWORD dwFlags
    );

typedef HRESULT (WINAPI * SwapChainPresent_T)(IDirect3DSwapChain9 * self,
                                              RECT * pSourceRect,
                                              CONST RECT * pDestRect,
                                              HWND hDestWindowOverride,
                                              CONST RGNDATA * pDirtyRegion,
                                              DWORD dwFlags);

}
}
}

#endif