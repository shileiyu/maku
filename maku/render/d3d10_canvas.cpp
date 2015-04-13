#include "d3d10_canvas.h"
#include "d3dx_shader.h"
#include "macro.h"
#include "utils.h"

namespace maku
{
namespace render
{
D3D10Canvas::D3D10Canvas(IDXGISwapChain * swapchain)
{
    swap_chain_ = swapchain;
    texture_ = NULL;
    const_buffer_ = NULL;
    ps_buffer_ = NULL;
    blend_state_ = NULL;
    stencil_state_ = NULL;
    vertex_buffer_ = NULL;
    resource_view_ = NULL;
    layout_ = NULL;
    vs_ = NULL;
    ps_ = NULL;
    target_view_ = NULL;
    ZeroMemory(&matrix_buffer_, sizeof(matrix_buffer_));
    ZeroMemory(&status_, sizeof(status_));
}

D3D10Canvas::~D3D10Canvas()
{
    swap_chain_ = NULL;
    SAFE_RELEASE(texture_);
    SAFE_RELEASE(vertex_buffer_);
    SAFE_RELEASE(resource_view_);
    SAFE_RELEASE(target_view_);
    SAFE_RELEASE(blend_state_);
    SAFE_RELEASE(stencil_state_);
    SAFE_RELEASE(layout_);
    SAFE_RELEASE(vs_);
    SAFE_RELEASE(ps_);
    SAFE_RELEASE(const_buffer_);
    SAFE_RELEASE(ps_buffer_);
}

bool D3D10Canvas::CreateBlendState(ID3D10Device * device)
{
    D3D10_BLEND_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.BlendEnable[0] = TRUE;
    desc.SrcBlend = D3D10_BLEND_SRC_ALPHA;
    desc.DestBlend = D3D10_BLEND_INV_SRC_ALPHA;
    desc.BlendOp = D3D10_BLEND_OP_ADD;
    desc.SrcBlendAlpha = D3D10_BLEND_ZERO;
    desc.DestBlendAlpha = D3D10_BLEND_ZERO;
    desc.BlendOpAlpha = D3D10_BLEND_OP_ADD;
    desc.RenderTargetWriteMask[0] = D3D10_COLOR_WRITE_ENABLE_ALL;
    return SUCCEEDED(device->CreateBlendState(&desc, &blend_state_));
}

bool D3D10Canvas::CreateBackBufferView(ID3D10Device * device)
{
    HRESULT hr;
    ID3D10Texture2D* pBuffer;
    hr = swap_chain_->GetBuffer( 0, __uuidof( ID3D10Texture2D ),
        ( LPVOID* )&pBuffer );
    if( FAILED( hr ) )
        return false;

    hr = device->CreateRenderTargetView( pBuffer, NULL, &target_view_ );
    pBuffer->Release();
    if( FAILED( hr ) )
        return false;

    return true;
}

void D3D10Canvas::CreateMatrix()
{
    matrix_buffer_.world = MatrixTranspose(MatrixIdentity());
    matrix_buffer_.view = MatrixTranspose(MatrixIdentity());
    matrix_buffer_.proj = MatrixTranspose(MatrixOrthographicOffCenterLH(0.0f,
        (float)rect_.width(), (float)rect_.height(), 0.0f, 0.0f, 100.0f));
}

bool D3D10Canvas::CreateConstantBuffer(ID3D10Device * device)
{
    HRESULT hr;
    D3D10_BUFFER_DESC buff_des;
    buff_des.Usage = D3D10_USAGE_DEFAULT;
    buff_des.ByteWidth = sizeof(MatrixBuffer);
    buff_des.BindFlags = D3D10_BIND_CONSTANT_BUFFER;
    buff_des.CPUAccessFlags = 0;
    buff_des.MiscFlags = 0;
    hr = device->CreateBuffer(&buff_des, NULL, &const_buffer_);
    if(FAILED(hr))
        return false;

    buff_des.ByteWidth = sizeof(ColorBuffer);
    hr = device->CreateBuffer(&buff_des, NULL, &ps_buffer_);
    if(FAILED(hr))
        return false;
    return true;
}

bool D3D10Canvas::CreateDepthStencilState(ID3D10Device * device)
{
    D3D10_DEPTH_STENCIL_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.DepthEnable = false;//关闭深度测试
    desc.DepthWriteMask = D3D10_DEPTH_WRITE_MASK_ALL;
    desc.DepthFunc = D3D10_COMPARISON_LESS;
    desc.StencilEnable = false;//关闭模板测试
    desc.StencilReadMask = 0xff;
    desc.StencilWriteMask = 0xff;

    return SUCCEEDED(device->CreateDepthStencilState(&desc,
        &stencil_state_));
}

bool D3D10Canvas::CreateInputLayout(ID3D10Device * device)
{
    D3D10_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
        0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
        12, D3D10_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElements = sizeof( layout ) / sizeof( layout[0] );
    // Create the input layout 
    return SUCCEEDED(device->CreateInputLayout(layout, numElements, 
        vs_shader, sizeof(vs_shader), &layout_));
}

bool D3D10Canvas::CreateShader(ID3D10Device * device)
{
    bool ret = SUCCEEDED(device->CreateVertexShader(vs_shader,
        sizeof(vs_shader), &vs_));
    if(!ret)
        return false;
    ret = SUCCEEDED(device->CreatePixelShader(ps_shader,
        sizeof(ps_shader), &ps_));
    return ret;
}

bool D3D10Canvas::CreateVertex(ID3D10Device * device)
{
    SimpleVertex vertex[] = 
    {
        SimpleVertex(0.0f, 0.0f, 0.5f, 0.0f, 0.0f),
        SimpleVertex((float)rect_.width(), 0.0f, 0.5f, 1.0f, 0.0f),
        SimpleVertex(0.0f, (float)rect_.height(), 0.5f, 0.0f, 1.0f),
        SimpleVertex((float)rect_.width(), (float)rect_.height(), 0.5f, 1.0f, 1.0f),
    };
    D3D10_BUFFER_DESC bd;
    bd.Usage = D3D10_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof( SimpleVertex ) * 4;
    bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
    bd.MiscFlags = 0;
    D3D10_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = vertex;

    return SUCCEEDED(device->CreateBuffer(&bd, &InitData, &vertex_buffer_));
}

bool D3D10Canvas::CreateTextureView(ID3D10Device * device,
    uint32_t width, uint32_t height)
{
    bool ret;
    D3D10_TEXTURE2D_DESC tDesc;
    ZeroMemory(&tDesc, sizeof(D3D10_TEXTURE2D_DESC));
    tDesc.Height = height;
    tDesc.Width = width;
    tDesc.Usage = D3D10_USAGE_DYNAMIC;
    tDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
    tDesc.MipLevels = 1;
    tDesc.ArraySize = 1;
    tDesc.SampleDesc.Count = 1;
    tDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    tDesc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
    tDesc.MiscFlags = 0;

    if( SUCCEEDED(device->CreateTexture2D(&tDesc, NULL, &texture_)) )
    {     
        ret = SUCCEEDED(device->CreateShaderResourceView(texture_,
            NULL, &resource_view_));
        if(!ret)
            SAFE_RELEASE(texture_);
    }
    return ret;
}

ID3D10Device * D3D10Canvas::GetDevice()
{
    HRESULT hr;
    ID3D10Device * device;
    hr = swap_chain_->GetDevice(__uuidof(ID3D10Device), (void**)&device);
    if(FAILED(hr))
        return NULL;
    device->Release();
    return device;
}

void D3D10Canvas::SaveStatus()
{
    ID3D10Device * device = GetDevice();
    if (device)
    {
        device->IAGetInputLayout(&status_.input_layout);
        device->IAGetVertexBuffers(0, 1, &status_.vertex_buffer,
            &status_.vertex_stride, &status_.vertex_offset);
        device->VSGetShader(&status_.vertex_shader);
        device->PSGetShader(&status_.pix_shader);
        device->VSGetConstantBuffers(0, 1, &status_.constant_buffer);
        device->PSGetConstantBuffers(1, 1, &status_.ps_buffer);
        device->IAGetPrimitiveTopology(
            (D3D10_PRIMITIVE_TOPOLOGY*)&status_.draw_style);
        device->PSGetShaderResources(0, 1, &status_.resource_view);
        device->OMGetBlendState(&status_.blend_state, status_.blen_factor,
            &status_.blen_mask);
        device->OMGetDepthStencilState(&status_.depth_state,
            &status_.stencil_ref);
        device->OMGetRenderTargets(1, &status_.target_view,
            &status_.depth_view);

        UINT number = D3D10_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
        device->RSGetViewports(&number, status_.view_port);
        number = D3D10_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;

        D3D10_RECT rect[D3D10_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
        device->RSGetScissorRects(&number, rect);
        device->RSGetState(&status_.rasterizer);
        Prepare(device);
    }
}

void D3D10Canvas::RestoreStatus()
{
    ID3D10Device * device = GetDevice();
    if (device)
    {
        device->IASetInputLayout(status_.input_layout);
        device->IASetVertexBuffers(0, 1, &status_.vertex_buffer,
            &status_.vertex_stride, &status_.vertex_offset);
        device->VSSetConstantBuffers(0, 1, &status_.constant_buffer);
        device->PSSetConstantBuffers(1, 1, &status_.ps_buffer);
        device->VSSetShader(status_.vertex_shader);
        device->PSSetShader(status_.pix_shader);
        device->IASetPrimitiveTopology(
            (D3D10_PRIMITIVE_TOPOLOGY)status_.draw_style);
        device->PSSetShaderResources(0, 1, &status_.resource_view);
        device->OMSetBlendState(status_.blend_state,
            status_.blen_factor, status_.blen_mask);
        device->OMSetDepthStencilState(status_.depth_state, status_.stencil_ref);
        device->OMSetRenderTargets(1, &status_.target_view, status_.depth_view);

        UINT number = D3D10_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
        device->RSSetViewports(number, status_.view_port);

        device->RSSetState(status_.rasterizer);
    }
    SAFE_RELEASE(status_.rasterizer);
    SAFE_RELEASE(status_.input_layout);
    SAFE_RELEASE(status_.vertex_buffer);
    SAFE_RELEASE(status_.vertex_shader);
    SAFE_RELEASE(status_.pix_shader);
    SAFE_RELEASE(status_.resource_view);
    SAFE_RELEASE(status_.depth_state);
    SAFE_RELEASE(status_.blend_state);
    SAFE_RELEASE(status_.target_view);
    SAFE_RELEASE(status_.depth_view);
}

void D3D10Canvas::Create(ID3D10Device * device)
{
    bool ret = true;
    DXGI_SWAP_CHAIN_DESC swap_desc;
    ZeroMemory(&swap_desc, sizeof(swap_desc));
    swap_chain_->GetDesc(&swap_desc);

    rect_.SetXYWH(0, 0, swap_desc.BufferDesc.Width,
        swap_desc.BufferDesc.Height);

    CreateMatrix();
    ret = CreateBackBufferView(device);
    ret = CreateBlendState(device);
    ret = CreateConstantBuffer(device);
    ret = CreateVertex(device);
    ret = CreateDepthStencilState(device);
    ret = CreateInputLayout(device);
    ret = CreateShader(device);
}

void D3D10Canvas::GetTextureWH(uint32_t & width, uint32_t & height)
{
    D3D10_TEXTURE2D_DESC tDesc;
    ZeroMemory(&tDesc, sizeof(D3D10_TEXTURE2D_DESC));
    if (texture_)
    {
        texture_->GetDesc(&tDesc);
        width = tDesc.Width;
        height = tDesc.Height;
    } 
    else
    {
        width = height = 0;
    }
}

void D3D10Canvas::Prepare(ID3D10Device * device)
{
    if (NULL == target_view_)
        Create(device);

    UINT stride = sizeof( SimpleVertex );
    UINT offset = 0;
    float blend_factor[4];
    blend_factor[0] = 0.0f;
    blend_factor[1] = 0.0f;
    blend_factor[2] = 0.0f;
    blend_factor[3] = 0.0f;

    device->IASetInputLayout(layout_);    
    device->IASetVertexBuffers( 0, 1, &vertex_buffer_, &stride, &offset );
    device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    device->UpdateSubresource(const_buffer_, 0, NULL, &matrix_buffer_, 0, 0);
    device->VSSetConstantBuffers(0, 1, &const_buffer_);
    device->VSSetShader(vs_);
    device->PSSetShader(ps_);    
    device->OMSetDepthStencilState(stencil_state_, 0);
    device->OMSetBlendState(blend_state_, blend_factor, 0xffffffff);
    device->RSSetState(0);   
    

    const uint32_t number = D3D10_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    D3D10_VIEWPORT view_port[number] = {0};
    view_port[0].Width = rect_.width();
    view_port[0].Height = rect_.height();
    view_port[0].TopLeftX = 0;
    view_port[0].TopLeftY = 0;
    view_port[0].MinDepth = 0.0f;
    view_port[0].MaxDepth = 1.0f;
    device->RSSetViewports(number, view_port);
    device->OMSetRenderTargets(1, &target_view_, NULL);
}

bool D3D10Canvas::UpdateVertexBuffer(SimpleVertex * sv, int n)
{
    HRESULT hr;
    void * data = NULL;
    hr = vertex_buffer_->Map(D3D10_MAP_WRITE_DISCARD, 0, &data);
    if(FAILED(hr))
        return false;

    SimpleVertex * p = reinterpret_cast<SimpleVertex*>(data);
    for (int i = 0; i < n; ++i)
        p[i] = sv[i];
    vertex_buffer_->Unmap();
    return true;
}

bool D3D10Canvas::UpdateTexture(Bitmap & bmp_info)
{
    HRESULT hr;
    D3D10_MAPPED_TEXTURE2D map_tex;
    hr = texture_->Map(0, D3D10_MAP_WRITE_DISCARD, 0, &map_tex);  
    char* src_buf = bmp_info.bits;
    
    if (SUCCEEDED(hr))
    {
        uint32_t pitch = map_tex.RowPitch;
        char* dst_buf = reinterpret_cast<char*>(map_tex.pData);
        //按图像数据拷贝
        uint32_t copy_width = pitch > bmp_info.pitch ? 
            bmp_info.pitch : pitch;
        uint32_t copy_height = bmp_info.height;

        for (uint32_t i = 0; i < copy_height; ++i)
        {
            memcpy_s(dst_buf + i*pitch, pitch, src_buf, copy_width);
            src_buf += copy_width;
        }
        texture_->Unmap(0);
    }
    else
    {
        SAFE_RELEASE(texture_);
        return false;
    }
    return true;
}

void D3D10Canvas::DrawBitmap(Bitmap & bmp_info, const Rect & dest)
{
    ID3D10Device * device = GetDevice();
    if (device)
    {
        if(rect_.isEmpty())
            return;
        //
        uint32_t width = 0;
        uint32_t height = 0;
        GetTextureWH(width, height);
        //create or update
        if (bmp_info.width > width || bmp_info.height > height)
        {
            //delete texture
            SAFE_RELEASE(texture_);
            SAFE_RELEASE(resource_view_);
            //recreate texture
            CreateTextureView(device, bmp_info.width, bmp_info.height);
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
        SimpleVertex cv_buffer[4];
        cv_buffer[0] = SimpleVertex((float)dest.left(),
            (float)dest.top(), 0.5f, 0.0f, 0.0f);
        cv_buffer[1] = SimpleVertex((float)dest.right(),
            (float)dest.top(), 0.5f, max_u, 0.0f);
        cv_buffer[2] = SimpleVertex((float)dest.left(),
            (float)dest.bottom(), 0.5f, 0.0f, max_v);
        cv_buffer[3] = SimpleVertex((float)dest.right(),
            (float)dest.bottom(), 0.5f, max_u, max_v);
        UpdateVertexBuffer(cv_buffer, 4);

        //set color buffer
        Float4 color;
        TransFloat(0, color);
        ColorBuffer color_buffer;
        color_buffer.color = color;
        device->UpdateSubresource(ps_buffer_, 0, NULL,
            &color_buffer, 0, 0);
        device->PSSetConstantBuffers(1, 1, &ps_buffer_);
        //draw
        device->PSSetShaderResources(0, 1, &resource_view_);
        device->Draw(4, 0);
    }
}

void D3D10Canvas::DrawRect(const Rect & rect, const Paint & paint)
{
    ID3D10Device * device = GetDevice();
    if (device)
    {
        ID3D10ShaderResourceView * temp = NULL;  
        SimpleVertex cv_buffer[4];
        cv_buffer[0] = SimpleVertex((float)rect.left(), 
            (float)rect.top(), 0.5f, 0.0f, 0.0f);
        cv_buffer[1] = SimpleVertex((float)rect.right(), 
            (float)rect.top(), 0.5f, 0.0f, 0.0f);
        cv_buffer[2] = SimpleVertex((float)rect.left(), 
            (float)rect.bottom(), 0.5f, 0.0f, 0.0f);
        cv_buffer[3] = SimpleVertex((float)rect.right(), 
            (float)rect.bottom(), 0.5f, 0.0f, 0.0f);
        UpdateVertexBuffer(cv_buffer, 4);

        Float4 color;
        TransFloat(paint.color, color);
        ColorBuffer color_buffer;
        color_buffer.color = color;
        device->UpdateSubresource(ps_buffer_, 0, NULL, &color_buffer, 0, 0);
        device->PSSetConstantBuffers(1, 1, &ps_buffer_);
        device->PSSetShaderResources(0, 1, &temp);
        device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
        device->Draw(4, 0);
    }
}

void D3D10Canvas::DrawLine(const Point & p1, 
                                                const Point & p2, 
                                                const Paint & paint)
{
    ID3D10Device * device = GetDevice();
    if (device)
    {
        ID3D10ShaderResourceView * temp = NULL;
        SimpleVertex sv[2];
        sv[0] = SimpleVertex(p1);
        sv[1] = SimpleVertex(p2);
        UpdateVertexBuffer(sv, 2);

        Float4 color;
        TransFloat(paint.color, color);
        ColorBuffer color_buffer;
        color_buffer.color = color;
        device->UpdateSubresource(ps_buffer_, 0, NULL, &color_buffer, 0, 0);
        device->PSSetConstantBuffers(1, 1, &ps_buffer_);
        device->PSSetShaderResources(0, 1, &temp);
        device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
        device->Draw(2, 0);
    }
}

void D3D10Canvas::DrawPoint(const Point & p1, const Paint & paint)
{
    ID3D10Device * device = GetDevice();
    if (device)
    {
        ID3D10ShaderResourceView * temp = NULL;
        SimpleVertex sv(p1);
        UpdateVertexBuffer(&sv, 1);

        Float4 color;
        TransFloat(paint.color, color);
        ColorBuffer color_buffer;
        color_buffer.color = color;
        device->UpdateSubresource(ps_buffer_, 0, NULL, &color_buffer, 0, 0);
        device->PSSetConstantBuffers(1, 1, &ps_buffer_);
        device->PSSetShaderResources(0, 1, &temp);
        device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);
        device->Draw(1, 0);
    }
}
}
}