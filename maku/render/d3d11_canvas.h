#ifndef MAKU_RENDER_D3D11_CANVAS_H_
#define MAKU_RENDER_D3D11_CANVAS_H_

#include "canvas.h"
#include <d3d11.h>

namespace maku
{

namespace render
{

class D3D11Canvas:public InternalCanvas
{
private:
    //vertex struct
    struct SimpleVertex
    {
        Float3 Pos;
        Float2 Tex;

        SimpleVertex(float x, float y, float z, float u, float v):
        Pos(x, y, z), Tex(u, v)
        {
            ;
        }

        SimpleVertex():Pos(0, 0, 0), Tex(0, 0)
        {
            ;
        }

        SimpleVertex(Point point):Tex(0, 0),
            Pos((float)point.x(), (float)point.y(), 0)
        {
            ;
        }
    };
    //constant buffer
    struct MatrixBuffer
    {
        Matrix world;
        Matrix view;
        Matrix proj;
    };
    //color
    struct ColorBuffer 
    {
        Float4 color;
    };
    //save status
    struct Status 
    {
        D3D11_VIEWPORT view_port[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
        ID3D11InputLayout * input_layout;
        ID3D11VertexShader * vertex_shader;
        ID3D11PixelShader * pixel_shader;
        ID3D11ShaderResourceView * shader_view;
        ID3D11Buffer * const_buffer;
        ID3D11Buffer * ps_buffer;
        ID3D11Buffer * vertex;//顶点缓存
        UINT vertex_stride;
        UINT vertex_offset;
        ID3D11DepthStencilState * depth_stencil_state;
        UINT stencil_ref;
        DWORD draw_style;//绘制单元
        ID3D11BlendState * blend_state;
        float blen_factor[4];
        UINT blen_mask;
        ID3D11RenderTargetView * render_target;
        ID3D11DepthStencilView * depth_stencil;
        ID3D11RasterizerState *  rasterizer;
    };
public:
    D3D11Canvas(IDXGISwapChain* swapchain);

    ~D3D11Canvas();

    void DrawPoint(const Point & p1, const Paint & paint);

    void DrawLine(const Point & p1, 
                            const Point & p2, 
                            const Paint & paint);

    void DrawRect(const Rect & rect, const Paint & paint);

    void DrawBitmap(Bitmap & bmp_info, const Rect & dest);

    void SaveStatus();

    void RestoreStatus();

    uint32_t GetWidth() 
    {
        return rect_.width();
    }

    uint32_t GetHeight() 
    {
        return rect_.height();
    }
private:
    void Prepare(ID3D11Device * device, ID3D11DeviceContext * context);

    bool Create(ID3D11Device * device);   

    void CreateMatrix();

    void GetTextureWH(uint32_t & width, uint32_t & height);

    ID3D11Device * GetDevice();

    bool UpdateTexture(ID3D11DeviceContext * context, Bitmap & bmp_info);

    bool CreateConstantBuffer(ID3D11Device * device);

    bool CreateBlendState(ID3D11Device * device);

    bool UpdateVertexBuffer(ID3D11DeviceContext * context,
        SimpleVertex * sv, int n);

    bool CreateDepthStencilState(ID3D11Device * device);

    bool CreateInputLayout(ID3D11Device * device);

    bool CreateShader(ID3D11Device * device); 

    bool CreateVertex(ID3D11Device * device);

    bool CreateTextureView(ID3D11Device * device, 
        uint32_t width, uint32_t height);

    bool CreateSamplerState(ID3D11Device * device);

    bool CreateBackBufferView(ID3D11Device * device);
private:
    Rect rect_;
    ID3D11Buffer* vertex_buffer_;
    IDXGISwapChain* swap_chain_;
    ID3D11Texture2D* texture_;
    ID3D11ShaderResourceView* resource_view_;
    ID3D11RenderTargetView* target_view_;
    ID3D11Buffer* const_buffer_;
    ID3D11Buffer* ps_buffer_;
    ID3D11BlendState* blend_state_;
    ID3D11SamplerState* sample_state_;
    ID3D11DepthStencilState* stencil_state_;
    ID3D11InputLayout* layout_;
    ID3D11VertexShader* vs_;
    ID3D11PixelShader* ps_;
    MatrixBuffer matrix_buffer_;
    Status status_;
};
}
}
#endif