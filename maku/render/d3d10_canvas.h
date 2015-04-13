#ifndef MAKU_RENDER_D3D10_CANVAS_H_
#define MAKU_RENDER_D3D10_CANVAS_H_

#include "canvas.h"
#include <d3d10.h>

namespace maku
{

namespace render
{
class D3D10Canvas:public InternalCanvas
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
    //
    struct ColorBuffer 
    {
        Float4 color;
    };
private:
    //save status
    struct Status 
    {
        D3D10_VIEWPORT view_port[D3D10_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
        ID3D10InputLayout* input_layout;
        ID3D10ShaderResourceView* resource_view;
        ID3D10VertexShader* vertex_shader;
        ID3D10PixelShader* pix_shader;
        ID3D10Buffer * vertex_buffer;
        ID3D10Buffer * constant_buffer;
        ID3D10Buffer * ps_buffer;
        UINT vertex_stride;
        UINT vertex_offset;
        ID3D10DepthStencilState* depth_state;
        UINT stencil_ref;
        DWORD draw_style;//绘制单元
        ID3D10BlendState* blend_state;
        float blen_factor[4];
        UINT blen_mask;
        ID3D10RenderTargetView * target_view;
        ID3D10DepthStencilView * depth_view;
        ID3D10RasterizerState *  rasterizer;
    };
public:
    D3D10Canvas(IDXGISwapChain * swapchain);

    ~D3D10Canvas();

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
    void Create(ID3D10Device * device);

    void Prepare(ID3D10Device * device);

    void CreateMatrix();

    ID3D10Device * GetDevice();

    void GetTextureWH(uint32_t & width, uint32_t & height);

    bool UpdateTexture(Bitmap & bmp_info);

    bool UpdateVertexBuffer(SimpleVertex * sv, int n);

    bool CreateConstantBuffer(ID3D10Device * device);

    bool CreateBlendState(ID3D10Device * device);

    bool CreateDepthStencilState(ID3D10Device * device);

    bool CreateInputLayout(ID3D10Device * device);

    bool CreateShader(ID3D10Device * device);

    bool CreateVertex(ID3D10Device * device);

    bool CreateTextureView(ID3D10Device * device,
        uint32_t width, uint32_t height);

    bool CreateBackBufferView(ID3D10Device * device);
private:
    Rect rect_;
    ID3D10Buffer* vertex_buffer_;
    IDXGISwapChain* swap_chain_;
    ID3D10Texture2D* texture_;
    ID3D10ShaderResourceView* resource_view_;
    ID3D10RenderTargetView* target_view_;
    ID3D10Buffer* const_buffer_;
    ID3D10Buffer* ps_buffer_;
    ID3D10BlendState* blend_state_;
    ID3D10DepthStencilState* stencil_state_;
    ID3D10InputLayout* layout_;
    ID3D10VertexShader* vs_;
    ID3D10PixelShader* ps_;
    MatrixBuffer matrix_buffer_;
    Status status_;
};
}
}
#endif