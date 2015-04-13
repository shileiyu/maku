#ifndef MAKU_RENDER_GDI_CANVAS_H_
#define MAKU_RENDER_GDI_CANVAS_H_

#include "canvas.h"

namespace maku
{

namespace render
{


namespace ddrawhooker
{
   
class GDICanvas:public InternalCanvas
{
public:
    GDICanvas();

    ~GDICanvas();

    void DrawPoint(const Point & p1, const Paint & paint);

    void DrawLine(const Point & p1, 
        const Point & p2, 
        const Paint & paint);

    void DrawRect(const Rect & rect, const Paint & paint);

    void DrawBitmap(Bitmap & bmp_info, const Rect & dest);

    void SaveStatus();

    void RestoreStatus();

    uint32_t GetWidth();

    uint32_t GetHeight();

    void SetRenderTarget(HDC hdc, uint32_t width, uint32_t height, const Rect &sub_rect);
private:
    void UpdateBitmap(Bitmap & bmp_info);

    void CreateBitmap(uint32_t width, uint32_t height, HBITMAP * bm, void ** data);

    HDC CreateHDC();

    void FreeSource();

    void FreeDestination();
private:
    HDC target_;
    Rect target_rect_;

    HBITMAP src_bitmap_;
    HGDIOBJ src_old_obj_;
    void * src_data_;
    HDC src_;
    uint32_t src_width_;
    uint32_t src_height_;

    HDC dest_;//目标DC
    uint32_t dest_width_;
    uint32_t dest_height_;
    HBITMAP dest_bitmap_;
    HGDIOBJ dest_old_obj_;
};

}


}


}
#endif