#ifndef MAKU_RENDER_CANVAS_H_
#define MAKU_RENDER_CANVAS_H_

#include <ncore/ncore.h>
#include "geometry.h"

namespace maku
{
namespace render
{

struct Bitmap
{
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    char * bits;
    Bitmap()
    {
        ::ZeroMemory(this, sizeof(Bitmap));
    }
};

struct Paint
{
    uint32_t color;
};

class Canvas
{
public:
    virtual void DrawPoint(const Point & p1, const Paint & paint) = 0;

    virtual void DrawLine(const Point & p1, 
                                       const Point & p2, 
                                       const Paint & paint) = 0;

    virtual void DrawRect(const Rect & rect, const Paint & paint) = 0;

    virtual void DrawBitmap(Bitmap & bmp_info, const Rect & dest) = 0;

    virtual uint32_t GetWidth() = 0;

    virtual uint32_t GetHeight() = 0;
};

class InternalCanvas:public Canvas
{
public:
    InternalCanvas()
    {
        ;
    }

    virtual ~InternalCanvas()
    {
        ;
    }

    virtual void SaveStatus() = 0;

    virtual void RestoreStatus() = 0;
};
}
}
#endif