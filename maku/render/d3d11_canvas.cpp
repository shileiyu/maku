#include "d3d11_canvas.h"
#include "d3dx_shader.h"
#include "utils.h"
#include "macro.h"

namespace maku
{
namespace render
{
D3D11Canvas::D3D11Canvas(IDXGISwapChain* swapchain)
{
    swap_chain_ = swapchain;
    texture_ = NULL;
    const_buffer_ = NULL;
    ps_buffer_ = NULL;
    blend_state_ = NULL;
    stencil_state_ = NULL;
    vertex_buffer_ = NULL;
    sample_state_ = NULL;
    resource_view_ = NULL;
    layout_ = NULL;
    vs_ = NULL;
    ps_ = NULL;
    target_view_ = NULL;
    ZeroMemory(&matrix_buffer_, sizeof(matrix_buffer_));
    ZeroMemory(&status_, sizeof(status_));
}

D3D11Canvas::~D3D11Canvas()
{
    swap_chain_ = NULL; 
    SAFE_RELEASE(texture_);
    SAFE_RELEASE(resource_view_);
    SAFE_RELEASE(vertex_buffer_);
    SAFE_RELEASE(target_view_);
    SAFE_RELEASE(blend_state_);   
    SAFE_RELEASE(stencil_state_);
    SAFE_RELEASE(sample_state_);
    SAFE_RELEASE(layout_);
    SAFE_RELEASE(const_buffer_);    
    SAFE_RELEASE(ps_buffer_);
    SAFE_RELEASE(vs_);
    SAFE_RELEASE(ps_);
}

bool D3D11Canvas::CreateBlendState(ID3D11Device * device)
{
    D3D11_BLEND_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.RenderTarget[0].BlendEnable = TRUE;
    desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
    desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    return SUCCEEDED(device->CreateBlendState(&desc, &blend_state_));
}

ID3D11Device * D3D11Canvas::GetDevice()
{
    HRESULT hr;
    ID3D11Device * temp = NULL;
    hr = swap_chain_->GetDevice(__uuidof(ID3D11Device), (void**)&temp);
    if(FAILED(hr))
        temp = NULL;

    temp->Release();
    return temp;
}

bool D3D11Canvas::Create(ID3D11Device * device)
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
    ret = CreateSamplerState(device);
    ret = CreateDepthStencilState(device);
    ret = CreateInputLayout(device);
    ret = CreateShader(device);
    return ret;
}

bool D3D11Canvas::UpdateTexture(ID3D11DeviceContext * context, 
                                Bitmap & bmp_info)
{
    HRESULT hr;
    D3D11_MAPPED_SUBRESOURCE map_tex;
    hr = context->Map(texture_, 0, D3D11_MAP_WRITE_DISCARD, 0, &map_tex);  
    char* src_buf = bmp_info.bits;

    if (SUCCEEDED(hr))
    {
        uint32_t pitch = map_tex.RowPitch;
        char* dst_buf = reinterpret_cast<char*>(map_tex.pData);
        uint32_t copy_width = pitch > bmp_info.pitch ? bmp_info.pitch : pitch;
        uint32_t copy_height = bmp_info.height;

        for (uint32_t i = 0; i < copy_height; ++i)
        {
            memcpy_s(dst_buf + i*pitch, pitch, src_buf, copy_width);
            src_buf += copy_width;
        }
        context->Unmap(texture_, 0);
    }
    else
    {
        SAFE_RELEASE(texture_);
        return false;
    }
    return true;
}

bool D3D11Canvas::CreateDepthStencilState(ID3D11Device * device)
{
    D3D11_DEPTH_STENCIL_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.DepthEnable = false;//关闭深度测试
    desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    desc.DepthFunc = D3D11_COMPARISON_LESS;
    desc.StencilEnable = false;//关闭模板测试
    desc.StencilReadMask = 0xff;
    desc.StencilWriteMask = 0xff;

    return SUCCEEDED(device->CreateDepthStencilState(&desc, &stencil_state_));
}

bool D3D11Canvas::CreateSamplerState(ID3D11Device * device)
{
    D3D11_SAMPLER_DESC samplerDesc;
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    // Create the texture sampler state.
    HRESULT result = device->CreateSamplerState(&samplerDesc, &sample_state_);
    if(FAILED(result))
        return false;

    return true;
}

bool D3D11Canvas::CreateConstantBuffer(ID3D11Device * device)
{
    HRESULT hr;
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(MatrixBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = device->CreateBuffer(&bd, NULL, &const_buffer_);
    if(FAILED(hr))
        return false;

    bd.ByteWidth = sizeof(ColorBuffer);
    hr = device->CreateBuffer(&bd, NULL, &ps_buffer_);
    if(FAILED(hr))
        return false;

    return true;
}

bool D3D11Canvas::CreateInputLayout(ID3D11Device * device)
{
    D3D11_INPUT_ELEMENT_DESC desc[] = 
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
        0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
        12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    uint32_t num = 2;
    return SUCCEEDED(device->CreateInputLayout(desc,
        num, vs_shader, sizeof(vs_shader), &layout_));
}

bool D3D11Canvas::CreateShader(ID3D11Device * device)
{
    bool ret = SUCCEEDED(device->CreateVertexShader(vs_shader,
        sizeof(vs_shader), NULL, &vs_));
    if(!ret)
        return false;

    ret = SUCCEEDED(device->CreatePixelShader(ps_shader,
        sizeof(ps_shader), NULL, &ps_));
    return ret;
}

void D3D11Canvas::GetTextureWH(uint32_t & width, uint32_t & height)
{
    D3D11_TEXTURE2D_DESC tDesc;
    ZeroMemory(&tDesc, sizeof(D3D11_TEXTURE2D_DESC));
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

bool D3D11Canvas::CreateTextureView(ID3D11Device * device,
    uint32_t width, uint32_t height)
{
    bool ret;
    D3D11_TEXTURE2D_DESC tDesc;
    ZeroMemory(&tDesc, sizeof(D3D11_TEXTURE2D_DESC));
    tDesc.Height = height;
    tDesc.Width = width;
    tDesc.Usage = D3D11_USAGE_DYNAMIC;
    tDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    tDesc.MipLevels = 1;
    tDesc.ArraySize = 1;
    tDesc.SampleDesc.Count = 1;
    tDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    tDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
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

bool D3D11Canvas::CreateVertex(ID3D11Device * device)
{
    SimpleVertex vertex[] = 
    {
        SimpleVertex(0.0f, 0.0f, 0.0f, 0.0f, 0.0f),
        SimpleVertex((float)rect_.width(), 0.0f, 0.0f, 1.0f, 0.0f),
        SimpleVertex(0.0f, (float)rect_.height(), 0.0f, 0.0f, 1.0f),
        SimpleVertex((float)rect_.width(),
         (float)rect_.height(), 0.0f,1.0f, 1.0f),
    };
    D3D11_BUFFER_DESC bd;
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof( SimpleVertex ) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = vertex;

    return SUCCEEDED(device->CreateBuffer(&bd, &InitData, &vertex_buffer_));
}

bool D3D11Canvas::CreateBackBufferView(ID3D11Device * device)
{
    HRESULT hr;
    ID3D11Texture2D * pBuffer;
    hr = swap_chain_->GetBuffer( 0, __uuidof( ID3D11Texture2D ),
        ( LPVOID* )&pBuffer );
    if( FAILED( hr ) )
        return false;

    hr = device->CreateRenderTargetView( pBuffer, NULL, &target_view_ );
    pBuffer->Release();
    if( FAILED( hr ) )
        return false;

    return true;
}

void D3D11Canvas::CreateMatrix()
{
    matrix_buffer_.world = MatrixTranspose(MatrixIdentity());
    matrix_buffer_.view = MatrixTranspose(MatrixIdentity());
    matrix_buffer_.proj = MatrixTranspose(MatrixOrthographicOffCenterLH(0.0f,
        (float)rect_.width(), (float)rect_.height(), 0.0f, 0.0f, 100.0f));
}

void D3D11Canvas::SaveStatus()
{
    ID3D11Device * device = GetDevice();
    if (device)
    {
        ID3D11DeviceContext * context = NULL;
        device->GetImmediateContext(&context);
        context->IAGetInputLayout(&status_.input_layout);
        context->IAGetVertexBuffers(0, 1, &status_.vertex,
            &status_.vertex_stride, &status_.vertex_offset);
        context->IAGetPrimitiveTopology(
            (D3D11_PRIMITIVE_TOPOLOGY *)&status_.draw_style);

        context->VSGetShader(&status_.vertex_shader, 0, 0);
        context->PSGetShader(&status_.pixel_shader, 0, 0);
        context->VSGetConstantBuffers(0, 1, &status_.const_buffer);
        context->PSGetConstantBuffers(1, 1, &status_.ps_buffer);
        context->PSGetShaderResources(0, 1, &status_.shader_view);
        context->OMGetDepthStencilState(&status_.depth_stencil_state,
            &status_.stencil_ref);
        context->OMGetBlendState(&status_.blend_state, status_.blen_factor, 
            &status_.blen_mask);
        context->OMGetRenderTargets(1, &status_.render_target,
            &status_.depth_stencil);

        UINT number = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
        context->RSGetViewports(&number, status_.view_port);

        number = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
        D3D11_RECT rect[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
        context->RSGetScissorRects(&number, rect);

        context->RSGetState(&status_.rasterizer);
        Prepare(device, context);
        context->Release();
    }
}

void D3D11Canvas::RestoreStatus()
{
    ID3D11Device * device = GetDevice();
    if (device)
    {
        ID3D11DeviceContext * context = NULL;
        device->GetImmediateContext(&context);
        context->IASetInputLayout(status_.input_layout);
        context->IASetVertexBuffers(0, 1, &status_.vertex,
            &status_.vertex_stride, &status_.vertex_offset);
        context->IASetPrimitiveTopology(
            (D3D11_PRIMITIVE_TOPOLOGY)status_.draw_style );
        context->VSSetShader(status_.vertex_shader, 0, 0);
        context->PSSetShader(status_.pixel_shader, 0, 0);
        context->VSSetConstantBuffers(0, 1, &status_.const_buffer);
        context->PSSetConstantBuffers(1, 1, &status_.ps_buffer);
        context->PSSetShaderResources(0, 1, &status_.shader_view);
        context->OMSetDepthStencilState(status_.depth_stencil_state,
            status_.stencil_ref);
        context->OMSetBlendState(status_.blend_state,
            status_.blen_factor, status_.blen_mask);
        context->OMSetRenderTargets(1, &status_.render_target,
            status_.depth_stencil);

        UINT number = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
        context->RSSetViewports(number, status_.view_port);

        context->RSSetState(status_.rasterizer);
        context->Release();
    }    
    SAFE_RELEASE(status_.rasterizer);
    SAFE_RELEASE(status_.input_layout);
    SAFE_RELEASE(status_.vertex);
    SAFE_RELEASE(status_.vertex_shader);
    SAFE_RELEASE(status_.pixel_shader);
    SAFE_RELEASE(status_.const_buffer);
    SAFE_RELEASE(status_.shader_view);
    SAFE_RELEASE(status_.depth_stencil_state);
    SAFE_RELEASE(status_.blend_state);
    SAFE_RELEASE(status_.render_target);
    SAFE_RELEASE(status_.depth_stencil);
}

bool D3D11Canvas::UpdateVertexBuffer(ID3D11DeviceContext * context,
    SimpleVertex * sv, int n)
{
    HRESULT hr;
    D3D11_MAPPED_SUBRESOURCE map_resource;
    hr = context->Map(vertex_buffer_, 0,
        D3D11_MAP_WRITE_DISCARD, 0, &map_resource);
    if(FAILED(hr))
        return false;

    void* data = map_resource.pData;
    SimpleVertex * p = reinterpret_cast<SimpleVertex*>(data);
    for (int i = 0; i < n; ++i)
        p[i] = sv[i];

    context->Unmap(vertex_buffer_, 0);
    return true;
}

void D3D11Canvas::Prepare(ID3D11Device * device, ID3D11DeviceContext * context)
{
    if (NULL == target_view_)
    {
        if (!Create(device))
            return;
    }
  
    const UINT vertex_stride = sizeof(SimpleVertex);
    const UINT vertex_offset = 0;

    context->IASetInputLayout(layout_);     

    context->IASetVertexBuffers(0, 1, &vertex_buffer_,
        &vertex_stride, &vertex_offset);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    context->UpdateSubresource(const_buffer_, 0, NULL, 
        &matrix_buffer_, 0,0);
    context->VSSetShader(vs_, NULL, 0);
    context->PSSetShader(ps_, NULL, 0);
    context->VSSetConstantBuffers(0, 1, &const_buffer_);
    context->OMSetDepthStencilState(stencil_state_, 0);//设置深度模板关闭

    float blen_factor[4];
    blen_factor[0] = 0.0f;
    blen_factor[1] = 0.0f;
    blen_factor[2] = 0.0f;
    blen_factor[3] = 0.0f;
    context->OMSetBlendState(blend_state_, blen_factor, 0xffffffff);
    context->RSSetState(0);//热血无赖 不加这句 窗口模式不能绘制
    context->OMSetRenderTargets(1, &target_view_, NULL);

    const uint32_t number = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    D3D11_VIEWPORT view_port[number] = {0};
    view_port[0].Width = (float)rect_.width();
    view_port[0].Height = (float)rect_.height();
    view_port[0].TopLeftX = 0;
    view_port[0].TopLeftY = 0;
    view_port[0].MinDepth = 0.0f;
    view_port[0].MaxDepth = 1.0f;
    context->RSSetViewports(number, view_port);
}

void D3D11Canvas::DrawRect(const Rect & rect, const Paint & paint)
{
    ID3D11ShaderResourceView* temp = NULL;  
    SimpleVertex cv_buffer[4];
    cv_buffer[0] = SimpleVertex((float)rect.left(), 
        (float)rect.top(), 0.5f, 0.0f, 0.0f);
    cv_buffer[1] = SimpleVertex((float)rect.right(), 
        (float)rect.top(), 0.5f, 0.0f, 0.0f);
    cv_buffer[2] = SimpleVertex((float)rect.left(), 
        (float)rect.bottom(), 0.5f, 0.0f, 0.0f);
    cv_buffer[3] = SimpleVertex((float)rect.right(), 
        (float)rect.bottom(), 0.5f, 0.0f, 0.0f);

    ID3D11Device * device = GetDevice();
    if (device)
    {
        ID3D11DeviceContext * context = NULL;
        device->GetImmediateContext(&context);
        UpdateVertexBuffer(context, cv_buffer, 4);

        Float4 color;
        TransFloat(paint.color, color);   
        ColorBuffer color_buffer;
        color_buffer.color = color;
        context->UpdateSubresource(ps_buffer_, 0, NULL,
            &color_buffer, 0, 0);
        context->PSSetConstantBuffers(1, 1, &ps_buffer_);
        context->PSSetShaderResources(0, 1, &temp);

        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        context->Draw(4, 0);
        context->Release();
    }
}

void D3D11Canvas::DrawBitmap(Bitmap & bmp_info, const Rect & dest)
{
    if(rect_.isEmpty())
        return;



    ID3D11Device * device = GetDevice();
    if (device)
    {
        ID3D11DeviceContext * context = NULL;
        device->GetImmediateContext(&context);
        
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
        if (!UpdateTexture(context, bmp_info))
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
        UpdateVertexBuffer(context, cv_buffer, 4);

        //set color buffer
        Float4 color;
        TransFloat(0, color);
        ColorBuffer color_buffer;
        color_buffer.color = color;
        context->UpdateSubresource(ps_buffer_, 0, NULL,
            &color_buffer, 0, 0);
        context->PSSetConstantBuffers(1, 1, &ps_buffer_);
        //draw
        context->PSSetShaderResources(0, 1, &resource_view_);
        context->Draw(4, 0);
        context->Release();
    }
}

void D3D11Canvas::DrawLine(const Point & p1, 
                                                const Point & p2, 
                                                const Paint & paint)
{
    ID3D11ShaderResourceView* temp = NULL;  
    SimpleVertex sv[2];
    sv[0] = SimpleVertex(p1);
    sv[1] = SimpleVertex(p2);
    ID3D11Device * device = GetDevice();
    if (device)
    {
        ID3D11DeviceContext * context = NULL;
        device->GetImmediateContext(&context);
        UpdateVertexBuffer(context, sv, 2);

        Float4 color;
        TransFloat(paint.color, color);
        ColorBuffer color_buffer;
        color_buffer.color = color;
        context->UpdateSubresource(ps_buffer_, 0, NULL,
            &color_buffer, 0, 0);
        context->PSSetConstantBuffers(1, 1, &ps_buffer_);
        context->PSSetShaderResources(0, 1, &temp);

        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        context->Draw(2, 0);
        context->Release();
    }
}

void D3D11Canvas::DrawPoint(const Point & p1, const Paint & paint)
{
    ID3D11ShaderResourceView* temp = NULL;
    SimpleVertex sv(p1);
    ID3D11Device * device = GetDevice();
    if (device)
    {
        ID3D11DeviceContext * context = NULL;
        device->GetImmediateContext(&context);
        UpdateVertexBuffer(context, &sv, 1);

        Float4 color;
        TransFloat(paint.color, color);
        ColorBuffer color_buffer;
        color_buffer.color = color;
        context->UpdateSubresource(ps_buffer_, 0, NULL,
            &color_buffer, 0, 0);
        context->PSSetConstantBuffers(1, 1, &ps_buffer_);
        context->PSSetShaderResources(0, 1, &temp);

        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
        context->Draw(1, 0);
        context->Release();
    }
}
}
}