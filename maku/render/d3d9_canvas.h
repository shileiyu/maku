#ifndef MAKU_RENDER_D3D9_DRAW_H_
#define MAKU_RENDER_D3D9_DRAW_H_

#include "canvas.h"
#include <d3d9.h>



namespace maku
{

namespace render
{

class D3D9Canvas:public InternalCanvas
{
private:
    struct SimpleVertex 
    {
        float _x, _y, _z, _w;
        DWORD color;
        float _u, _v;

        static DWORD GetFVF() 
        {
            return D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;
        }

        static uint32_t GetSize() 
        {
            return sizeof(SimpleVertex);
        }

        SimpleVertex() 
        {
            ZeroMemory(this, sizeof(SimpleVertex));
        }

        SimpleVertex& operator = (const SimpleVertex& other)
        {
            _x = other._x; _y = other._y; _z = other._z; _u = other._u;
            _v = other._v; _w = other._w; color = other.color;
            return *this;
        }

        SimpleVertex(Point point, uint32_t value)
        {
            _x = (float)point.x(); _y = (float)point.y(); _z = _u = _v = 0.0f;
            _w = 1.0f; color = value;
        }

        SimpleVertex(float x, float y, float z, uint32_t value)
        {
            _x = x; _y = y; _z =z;  _u = _v = 0.0f;
            _w = 1.0f; color = value;
        }

        SimpleVertex(float x, float y, float z, float w, float u, float v)
        {
            _x = x; _y = y; _z = z; _w = 1.0f; _u = u; _v = v;
            color = 0;
        }
    };
public:
    D3D9Canvas(IDirect3DDevice9 * device);

    ~D3D9Canvas();

    uint32_t GetCountRef();

    void DrawPoint(const Point & p1, const Paint & paint);

    void DrawLine(const Point & p1, 
                            const Point & p2, 
                            const Paint & paint);

    void DrawRect(const Rect & rect, const Paint & paint);

    void DrawBitmap(Bitmap & bmp_info, const Rect & dest);

    void UpdateRenderTarget(IDirect3DSurface9 * target, RECT & rect);

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
    void Prepare();

    void SetTextureState();

    void SetDiffuseState(); 

    void CreateTexture(uint32_t width, uint32_t height);

    bool UpdateTexture(Bitmap & bmp_info);

    void GetTextureWH(uint32_t & width, uint32_t & height);
private:
    IDirect3DDevice9 * device9_;
    LPDIRECT3DTEXTURE9 texture_;
    Rect rect_;
    IDirect3DStateBlock9 * state_block_;
    IDirect3DSurface9 * canvas_target_;//不占用对Device的引用计数

    IDirect3DSurface9 * last_render_target_;
};
}
}
#endif