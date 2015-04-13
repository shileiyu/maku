#ifndef MAKU_RENDER_DXGI_COMMON_H_
#define MAKU_RENDER_DXGI_COMMON_H_

//d3d11 headers will refer to windows header.
//without this macro that will lead to conflict with winsock2 header
//we define it ourself.
#include <ncore/ncore.h>
#include <d3d11.h>

namespace maku
{

namespace render
{

namespace dxgihooker
{

//IUnknown
static const size_t kIdxRelease                 = 0x02;
//IDXGISwapChain
static const size_t kIdxSwapChainPresent        = 0x08;
static const size_t kIdxSwapChainResizeBuffers  = 0x0D;
static const size_t kIdxSwapChainResizeTarget   = 0x0E;
//IDXGIFactory
static const size_t kIdxCreateSwapChain         = 0x0A;

typedef HRESULT (WINAPI * CreateSwapChain_T)(IDXGIFactory *self,
                                             IUnknown *pDevice,
                                             DXGI_SWAP_CHAIN_DESC *pDesc,
                                             IDXGISwapChain **ppSwapChain);

typedef ULONG (WINAPI * Release_T)(IUnknown * self);

typedef HRESULT (WINAPI * CreateDXGIFactory_T)(
    _In_   REFIID riid,
    _Out_  void **ppFactory
    );

typedef HRESULT (WINAPI * CreateDXGIFactory1_T)(
    _In_   REFIID riid,
    _Out_  void **ppFactory
    );

typedef HRESULT (WINAPI * SwapChainPresent_T)(IDXGISwapChain *self,
                                              UINT SyncInterval,
                                              UINT Flags);

typedef HRESULT (WINAPI * SwapChainResizeBuffers_T)(IDXGISwapChain *self,
    UINT BufferCount,
    UINT Width,
    UINT Height,
    DXGI_FORMAT NewFormat,
    UINT SwapChainFlags);

typedef HRESULT (WINAPI * SwapChainResizeTarget_T)(IDXGISwapChain *self,
    const DXGI_MODE_DESC *pNewTargetParameters);
}
}
}

#endif