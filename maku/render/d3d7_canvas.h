#ifndef MAKU_RENDER_D3D7_DRAW_H_
#define MAKU_RENDER_D3D7_DRAW_H_

#include "canvas.h"
#include <ddraw.h>
#include "d3d7\d3d7.h"


namespace maku
{

namespace render
{

namespace ddrawhooker
{

class D3D7Canvas:public InternalCanvas
{
private:
    struct SimpleVertex 
    {
        float _x, _y, _z, _w;
        DWORD color;
        float  _u, _v;

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
            _x = (float)point.x(); _y = (float)point.y(); _z =1.0f; _u = _v = 0.0f;
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
    D3D7Canvas(IDirect3DDevice7 * device);

    ~D3D7Canvas();

    IDirect3DDevice7* GetDevice()
    {
        return device7_;
    }

    void DrawPoint(const Point & p1, const Paint & paint);

    void DrawLine(const Point & p1, 
                            const Point & p2, 
                            const Paint & paint);

    void DrawRect(const Rect & rect, const Paint & paint);

    void DrawBitmap(Bitmap & bmp_info, const Rect & dest);
    //get ddraw from device7
    IDirectDraw7* GetDirectDraw();

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

    void SetRenderTarget(LPDIRECTDRAWSURFACE7 back_buffer, const Rect & rect);
private:
    void Prepare();

    void SetTextureState();

    void SetDiffuseState(); 

    void CreateTexture(uint32_t width, uint32_t height);

    bool UpdateTexture(Bitmap & bmp_info);
    //get texture width and height
    void GetTextureWH(uint32_t & width, uint32_t & height);
private:
    IDirect3DDevice7 * device7_;
    LPDIRECTDRAWSURFACE7 texture_;
    
    LPDIRECTDRAWSURFACE7 back_buffer_;
    LPDIRECTDRAWSURFACE7 last_render_target_;
    LPDIRECTDRAWSURFACE7 last_texture_;
    Rect rect_;
    DWORD state_block_;
};
}
}
}
#endif