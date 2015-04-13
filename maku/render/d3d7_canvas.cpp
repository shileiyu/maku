#include "d3d7_canvas.h"
#include "ddraw_common.h"
#include "macro.h"
#include "utils.h"

namespace maku
{
namespace render
{
namespace ddrawhooker
{
const GUID IID_IDirect3DDevice7_T = {0xf5049e79, 0x4861, 0x11d2, 0xa4, 0x7, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0xa8};
D3D7Canvas::D3D7Canvas(IDirect3DDevice7 * device)
{
    device7_ = device;
    state_block_ = 0;
    texture_ = NULL;
    back_buffer_ = NULL;
    last_render_target_ = NULL;
    last_texture_ = NULL;
}

D3D7Canvas::~D3D7Canvas()
{
    device7_ = NULL;
    SAFE_RELEASE(texture_);
}

void D3D7Canvas::CreateTexture(uint32_t width, uint32_t height)
{
    DDSURFACEDESC2 surfaceDesc;
    ::ZeroMemory(&surfaceDesc, sizeof(surfaceDesc));
    surfaceDesc.dwSize = sizeof(surfaceDesc);
    surfaceDesc.dwFlags = DDSD_CAPS | DDSD_WIDTH |
        DDSD_HEIGHT | DDSD_PIXELFORMAT ;
    surfaceDesc.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_TEXTURE | DDSCAPS_3DDEVICE;
    surfaceDesc.dwWidth = width;
    surfaceDesc.dwHeight = height;
    surfaceDesc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    surfaceDesc.ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
    surfaceDesc.ddpfPixelFormat.dwRGBBitCount = 32;
    surfaceDesc.ddpfPixelFormat.dwRBitMask = 0x00ff0000;
    surfaceDesc.ddpfPixelFormat.dwGBitMask = 0x0000ff00;
    surfaceDesc.ddpfPixelFormat.dwBBitMask = 0x000000ff;
    surfaceDesc.ddpfPixelFormat.dwRGBAlphaBitMask = 0xff000000;

    HRESULT hr = GetDirectDraw()->CreateSurface(&surfaceDesc, &texture_, NULL);
    if (hr != D3D_OK)
        texture_ = NULL;
}

IDirectDraw7* D3D7Canvas::GetDirectDraw()
{
    IDirect3D7* dd7 = NULL;
    IDirectDraw7* ddraw = NULL;

    device7_->GetDirect3D(&dd7);
    dd7->QueryInterface(IID_IDirectDraw7_T, (void**)&ddraw);

    if (ddraw != NULL)
    {
        dd7->Release();
        ddraw->Release();
    }
    return ddraw;
}

void D3D7Canvas::GetTextureWH(uint32_t & width, uint32_t & height)
{
    if (texture_)
    {
        DDSURFACEDESC2 surfaceDesc;
        ::ZeroMemory(&surfaceDesc, sizeof(surfaceDesc));
        surfaceDesc.dwSize = sizeof(surfaceDesc);
        texture_->GetSurfaceDesc(&surfaceDesc);
        width = surfaceDesc.dwWidth;
        height = surfaceDesc.dwHeight;
    }
    else
    {
        width = 0;
        height = 0;
    }
}

bool D3D7Canvas::UpdateTexture(Bitmap & bmp_info)
{
    DDSURFACEDESC2 surfaceDesc;
    ::ZeroMemory(&surfaceDesc, sizeof(surfaceDesc));
    surfaceDesc.dwSize = sizeof(surfaceDesc);
  
    HRESULT hr = texture_->Lock(NULL, &surfaceDesc, DDLOCK_WAIT, NULL);
    char* src_buf = bmp_info.bits;

    if (SUCCEEDED(hr))
    {
        uint32_t pitch = surfaceDesc.lPitch;
        uint32_t height = surfaceDesc.dwHeight;

        char* dst_buf = reinterpret_cast<char*>(surfaceDesc.lpSurface);
        uint32_t copy_width = pitch > bmp_info.pitch ?
            bmp_info.pitch : pitch;
        uint32_t copy_height = height > bmp_info.height ? 
                               bmp_info.height : height;

        for (uint32_t i = 0; i < copy_height; ++i)
        {
            memcpy_s(dst_buf + i*pitch, pitch, src_buf, copy_width);
            src_buf += copy_width;
        }
        texture_->Unlock(NULL);
    }
    else
    {
        SAFE_RELEASE(texture_);
        return false;
    }
    return true;
}

void D3D7Canvas::SetRenderTarget(LPDIRECTDRAWSURFACE7 back_buffer, const Rect & rect)
{
    back_buffer_ = back_buffer;
    rect_ = rect;
}

void D3D7Canvas::SaveStatus()
{
    device7_->CreateStateBlock(D3DSBT_ALL, &state_block_);
    device7_->CaptureStateBlock(state_block_);

    device7_->GetRenderTarget(&last_render_target_);
    device7_->GetTexture(0, &last_texture_);//sacred.exe 
    //
    Prepare();
}

void D3D7Canvas::RestoreStatus()
{
    device7_->ApplyStateBlock(state_block_);
    device7_->DeleteStateBlock(state_block_);

    device7_->SetRenderTarget(last_render_target_, 0);
    SAFE_RELEASE(last_render_target_);
    SAFE_RELEASE(last_texture_);
}

void D3D7Canvas::Prepare()
{
    device7_->SetTextureStageState(0,  D3DTSS_ADDRESS,   D3DTADDRESS_WRAP);
    device7_->SetTextureStageState(0,  D3DTSS_MAGFILTER, D3DTFG_POINT);
    device7_->SetTextureStageState(0,  D3DTSS_MINFILTER, D3DTFG_POINT);
    device7_->SetTextureStageState(0,  D3DTSS_MIPFILTER, D3DTFP_NONE);
    device7_->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
    device7_->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_SRCALPHA);
    device7_->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
    device7_->SetRenderState(D3DRENDERSTATE_CLIPPLANEENABLE, FALSE);
    device7_->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);
    device7_->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
    device7_->SetRenderState(D3DRENDERSTATE_CULLMODE, 3);
}

void D3D7Canvas::SetTextureState()
{
    device7_->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);					
    device7_->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    device7_->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);	
    device7_->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    device7_->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
}

void D3D7Canvas::SetDiffuseState()
{
    device7_->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
    device7_->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);	
    device7_->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    device7_->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
}

void D3D7Canvas::DrawRect(const Rect & rect, const Paint & paint)
{
    SimpleVertex cv_buffer[4];
    cv_buffer[0] = SimpleVertex((float)rect.left(), 
        (float)rect.top(), 0.5f, paint.color);
    cv_buffer[1] = SimpleVertex((float)rect.right(), 
        (float)rect.top(), 0.5f, paint.color);
    cv_buffer[2] = SimpleVertex((float)rect.left(), 
        (float)rect.bottom(), 0.5f, paint.color);
    cv_buffer[3] = SimpleVertex((float)rect.right(), 
        (float)rect.bottom(), 0.5f, paint.color);

    SetDiffuseState();
    device7_->DrawPrimitive(D3DPT_TRIANGLESTRIP, SimpleVertex::GetFVF(),
        (LPVOID)cv_buffer, 4, 0);
}

void D3D7Canvas::DrawBitmap(Bitmap & bmp_info, const Rect & dest)
{
    if (NULL == back_buffer_)
        return;
    HRESULT hr = device7_->SetRenderTarget(back_buffer_, 0);
    if(FAILED(hr))
        return;
    DDSURFACEDESC2 surfaceDesc;
    ::ZeroMemory(&surfaceDesc, sizeof(surfaceDesc));
    surfaceDesc.dwSize = sizeof(surfaceDesc);
    back_buffer_->GetSurfaceDesc(&surfaceDesc);

    D3DVIEWPORT7 port = {0, 0, surfaceDesc.dwWidth, 
        surfaceDesc.dwHeight, 
        0.0f, 1.0f};
    hr = device7_->SetViewport(&port);
    if(FAILED(hr))
        return;

    //set state
    SetTextureState();

    uint32_t width = 0;
    uint32_t height = 0;
    GetTextureWH(width, height);
    //create or update
    if (bmp_info.width > width || bmp_info.height > height)
    {
        //delete texture
        if (texture_)
            SAFE_RELEASE(texture_);      
        //recreate texture
        CreateTexture(bmp_info.width, bmp_info.height);
    }
    //
    if (!UpdateTexture(bmp_info))
        return;

    GetTextureWH(width, height);
    if(width == 0 || height == 0)
        return;
    //update vertex data
    float max_u = (float)(bmp_info.width) / width;
    float max_v = (float)(bmp_info.height) / height;

    SimpleVertex vertex_buffer[4];
    vertex_buffer[0] = SimpleVertex(-0.5f + dest.left() + rect_.left(),
        -0.5f + dest.top() + rect_.top(), 0.0f, 1.0f, 0.0f, 0.0f);
    vertex_buffer[1] = SimpleVertex(-0.5f + dest.right() + rect_.left(),
        -0.5f + dest.top() + rect_.top(), 0.0f, 1.0f, max_u, 0.0f);
    vertex_buffer[2] = SimpleVertex(-0.5f + dest.left() + rect_.left(),
        -0.5f + dest.bottom() + rect_.top(), 0.0f, 1.0f, 0.0f, max_v);
    vertex_buffer[3] = SimpleVertex(-0.5f + dest.right() + rect_.left(),
        -0.5f + dest.bottom() + rect_.top(), 0.0f, 1.0f, max_u, max_v);

    //draw
    device7_->SetTexture(0, texture_);
    device7_->DrawPrimitive(D3DPT_TRIANGLESTRIP, SimpleVertex::GetFVF(), 
        (LPVOID)vertex_buffer, 4, 0);
}

void D3D7Canvas::DrawLine(const Point & p1, 
                                              const Point & p2, 
                                              const Paint & paint)
{
    SimpleVertex cv_buffer[2];
    cv_buffer[0] = SimpleVertex(p1, paint.color);
    cv_buffer[1] = SimpleVertex(p2, paint.color);

    SetDiffuseState();    
    device7_->DrawPrimitive(D3DPT_LINELIST, SimpleVertex::GetFVF(),
        (LPVOID)cv_buffer, 2, 0);
}

void D3D7Canvas::DrawPoint(const Point & p1, const Paint & paint)
{
    SimpleVertex sv(p1, paint.color);

    SetDiffuseState();   
    device7_->DrawPrimitive(D3DPT_POINTLIST, SimpleVertex::GetFVF(),
        (LPVOID)&sv, 1, 0);
}
}
}
}