#include "d3d9_canvas.h"
#include "utils.h"
#include "macro.h"

namespace maku
{
namespace render
{
D3D9Canvas::D3D9Canvas(IDirect3DDevice9 * device)
{
    device9_ = device;
    texture_ = NULL;
    state_block_ = NULL;
    canvas_target_ = NULL;
    last_render_target_ = NULL;

    LPDIRECT3DSWAPCHAIN9 swap_chain = NULL;
    device9_->GetSwapChain(0, &swap_chain); 
    swap_chain->Release();

    D3DPRESENT_PARAMETERS desc;
    swap_chain->GetPresentParameters(&desc); 

    rect_.SetXYWH(0, 0, desc.BackBufferWidth,
        desc.BackBufferHeight);
}

D3D9Canvas::~D3D9Canvas()
{
    SAFE_RELEASE(texture_);
    device9_ = NULL;
}

void D3D9Canvas::CreateTexture(uint32_t width, uint32_t height)
{    
    HRESULT hr = device9_->CreateTexture(width, height, 1,
        D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8,
        D3DPOOL_DEFAULT, &texture_, NULL);
   
    if(FAILED(hr))
    {
        texture_ = NULL;
        return ;
    }
}

uint32_t D3D9Canvas::GetCountRef()
{
    uint32_t ret = 0;
    if (texture_)
        ret++;

    return ret;
}

void D3D9Canvas::UpdateRenderTarget(IDirect3DSurface9 * surface, RECT & rect)
{
    TransRect(rect, rect_);
    canvas_target_ = surface;
}

bool D3D9Canvas::UpdateTexture(Bitmap & bmp_info)
{
    D3DLOCKED_RECT lock_rect;
    RECT rect;
    rect.top = 0;
    rect.left = 0;
    rect.right = bmp_info.width;
    rect.bottom = bmp_info.height;
    HRESULT hr = texture_->LockRect(0, &lock_rect, &rect, 0);
    char* src_buf = bmp_info.bits;

    if (SUCCEEDED(hr))
    {
        uint32_t pitch = lock_rect.Pitch;
        char* dst_buf = reinterpret_cast<char*>(lock_rect.pBits);
        uint32_t copy_width = pitch > bmp_info.pitch ?
            bmp_info.pitch : pitch;
        uint32_t copy_height = bmp_info.height;

        for (uint32_t i = 0; i < copy_height; ++i)
        {
            memcpy_s(dst_buf + i*pitch, pitch, src_buf, copy_width);
            src_buf += copy_width;
        }
        texture_->UnlockRect(0);
    } 
    else
    {
        SAFE_RELEASE(texture_);
        return false;
    }
    return true;
}

void D3D9Canvas::GetTextureWH(uint32_t & width, uint32_t & height)
{
    if (texture_)
    {
        D3DSURFACE_DESC surface_desc;
        ZeroMemory(&surface_desc, sizeof(surface_desc));
        texture_->GetLevelDesc(0, &surface_desc);
        width = surface_desc.Width;
        height = surface_desc.Height;
    } 
    else
    {
        width = 0;
        height = 0;
    }
}

void D3D9Canvas::Prepare()
{
    device9_->SetPixelShader(NULL);
    device9_->SetVertexShader(NULL);

    device9_->SetNPatchMode(0.0f);
    device9_->SetRenderState(D3DRS_CLIPPING, FALSE);
    device9_->SetRenderState(D3DRS_ZENABLE, FALSE);
    device9_->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    device9_->SetRenderState(D3DRS_LIGHTING, FALSE);
    device9_->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    device9_->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
    device9_->SetRenderState(D3DRS_FOGENABLE, FALSE);
    device9_->SetRenderState(D3DRS_STENCILENABLE, FALSE);	

    device9_->SetRenderState(D3DRS_DITHERENABLE, FALSE)	;
    device9_->SetRenderState(D3DRS_SPECULARENABLE, FALSE)	;
    device9_->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);	
    device9_->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, FALSE);
    device9_->SetRenderState(D3DRS_NORMALIZENORMALS, FALSE);	
    device9_->SetRenderState(D3DRS_CLIPPLANEENABLE, 0);

    device9_->SetRenderState(D3DRS_CULLMODE,			D3DCULL_CCW);
    device9_->SetRenderState(D3DRS_ALPHABLENDENABLE,	TRUE);
    device9_->SetRenderState(D3DRS_SRCBLEND,			D3DBLEND_SRCALPHA);
    device9_->SetRenderState(D3DRS_DESTBLEND,			D3DBLEND_INVSRCALPHA);
    device9_->SetRenderState(D3DRS_BLENDOP,             D3DBLENDOP_ADD);
    device9_->SetRenderState(D3DRS_SRGBWRITEENABLE, FALSE);
    //设置渲染目标
    device9_->SetRenderTarget(0, canvas_target_);
}

void D3D9Canvas::SetTextureState()
{
    device9_->SetSamplerState(0,  D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP);
    device9_->SetSamplerState(0,  D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP);
    device9_->SetSamplerState(0,  D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    device9_->SetSamplerState(0,  D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    device9_->SetSamplerState(0,  D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    device9_->SetSamplerState(0,  D3DSAMP_SRGBTEXTURE, FALSE);

    device9_->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);					
    device9_->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    device9_->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);	
    device9_->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    device9_->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
}

void D3D9Canvas::SetDiffuseState()
{
    device9_->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
    device9_->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);	
    device9_->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    device9_->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
}

void D3D9Canvas::SaveStatus()
{
    device9_->GetRenderTarget(0, &last_render_target_);
    device9_->CreateStateBlock(D3DSBT_ALL, &state_block_);
    state_block_->Capture();

    Prepare(); 
}

void D3D9Canvas::RestoreStatus()
{
    state_block_->Apply();
    SAFE_RELEASE(state_block_);
    device9_->SetRenderTarget(0, last_render_target_);
    SAFE_RELEASE(last_render_target_);
}

void D3D9Canvas::DrawBitmap(Bitmap & bmp_info, const Rect & dest)
{
    if(rect_.isEmpty())
        return;
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
    //update data
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
    device9_->SetTexture(0, texture_);
    device9_->SetFVF(SimpleVertex::GetFVF());
    device9_->BeginScene();
    device9_->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2,
        (void*)vertex_buffer, SimpleVertex::GetSize());
    device9_->EndScene();
}

void D3D9Canvas::DrawPoint(const Point & p1, const Paint & paint)
{
    SimpleVertex sv(p1, paint.color);

    SetDiffuseState();
    device9_->SetFVF(SimpleVertex::GetFVF());
    device9_->BeginScene();
    device9_->DrawPrimitiveUP(D3DPT_POINTLIST, 1,
        (LPVOID)&sv, SimpleVertex::GetSize());
    device9_->EndScene();
}

void D3D9Canvas::DrawRect(const Rect & rect, const Paint & paint)
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
    device9_->SetFVF(SimpleVertex::GetFVF());
    device9_->BeginScene();
    device9_->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2,
        (LPVOID)cv_buffer, SimpleVertex::GetSize());
    device9_->EndScene();
}

void D3D9Canvas::DrawLine(const Point & p1, 
                                              const Point & p2, 
                                              const Paint & paint)
{
    SimpleVertex cv_buffer[2];
    cv_buffer[0] = SimpleVertex(p1, paint.color);
    cv_buffer[1] = SimpleVertex(p2, paint.color);  

    SetDiffuseState();
    device9_->SetFVF(SimpleVertex::GetFVF());
    device9_->BeginScene();
    device9_->DrawPrimitiveUP(D3DPT_LINELIST, 1,
        (LPVOID)cv_buffer, SimpleVertex::GetSize());
    device9_->EndScene();
}
}
}