#include "gdi_canvas.h"


namespace maku
{
namespace render
{

namespace ddrawhooker
{

GDICanvas::GDICanvas()
    :src_bitmap_(0), src_old_obj_(0), src_data_(0), 
    src_(0), src_width_(0), src_height_(0), target_(0),
    dest_(0), dest_width_(0), dest_height_(0), dest_bitmap_(0),
    dest_old_obj_(0)
{
    
}

GDICanvas::~GDICanvas()
{
    FreeDestination();
    FreeSource();
}

void GDICanvas::SetRenderTarget(HDC hdc, uint32_t width, uint32_t height, const Rect &sub_rect)
{
    if(dest_height_ != height || dest_width_ != width)
    {
        FreeDestination();
        dest_ = CreateHDC();
        CreateBitmap(width, height, &dest_bitmap_, 0);
        dest_old_obj_ = ::SelectObject(dest_, dest_bitmap_);
        dest_height_ = height;
        dest_width_ = width;
    }
    target_ = hdc;
    target_rect_ = sub_rect;
}

uint32_t GDICanvas::GetHeight()
{
    return target_rect_.height();
}

uint32_t GDICanvas::GetWidth()
{
    return target_rect_.width();
}
void GDICanvas::DrawPoint(const Point & p1, const Paint & paint)
{
    ;
}

void GDICanvas::DrawLine(const Point & p1, 
    const Point & p2, 
    const Paint & paint)
{
    ;
}

void GDICanvas::DrawRect(const Rect & rect, const Paint & paint)
{
    ;
}

void GDICanvas::DrawBitmap(Bitmap & bmp_info, const Rect & dest)
{
    if(0 == target_ || target_rect_.isEmpty())
        return;

    UpdateBitmap(bmp_info);

    BLENDFUNCTION blend;
    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = 0;
    blend.AlphaFormat = AC_SRC_ALPHA;
    blend.SourceConstantAlpha = 255;

    BOOL ret = ::AlphaBlend(
        dest_, 
        target_rect_.left() + dest.left(),
        target_rect_.top() + dest.top(),
        dest.width(),
        dest.height(),
        src_,
        0,0, bmp_info.width, bmp_info.height, blend);
    if(!ret)
    {
        DWORD error = ::GetLastError();
        assert(ret);
    }
}

void GDICanvas::SaveStatus()
{
    ::BitBlt(dest_, 0, 0, dest_width_, dest_height_, target_, 0, 0, SRCCOPY);
}

void GDICanvas::RestoreStatus()
{
    ::BitBlt(target_, 0, 0, dest_width_, dest_height_, dest_, 0, 0, SRCCOPY);
}

void GDICanvas::UpdateBitmap(Bitmap & bmp_info)
{
    if(bmp_info.width > src_width_ || bmp_info.height > src_height_)
    {
        FreeSource();
        CreateBitmap(bmp_info.width, bmp_info.height, &src_bitmap_, &src_data_);
        src_ = CreateHDC();
        src_old_obj_ = ::SelectObject(src_, src_bitmap_);
        src_width_ = bmp_info.width;
        src_height_ = bmp_info.height;
        if(0 == src_data_)
            return;
    }
    //复制像素
    uint32_t pitch = 4 * src_width_;
    uint32_t height = src_height_;
    char* dst_buf = reinterpret_cast<char*>(src_data_);

    char * src_buf = bmp_info.bits;
    uint32_t copy_width = pitch > bmp_info.pitch ?
        bmp_info.pitch : pitch;
    uint32_t copy_height = height > bmp_info.height ? 
        bmp_info.height : height;

    for (uint32_t i = 0; i < copy_height; ++i)
    {
        memcpy_s(dst_buf + i*pitch, pitch, src_buf, copy_width);
        src_buf += bmp_info.pitch;
    }
    //预乘ALPHA
    //for(uint32_t i = 0; i < copy_width / 4; i++)
    //{
    //    for(uint32_t j = 0; j < copy_height; j++)
    //    {
    //        char * byte = (char *)(dst_buf + j * pitch + i);
    //        byte[0] = byte[0] * byte[3] / 255;
    //        byte[1] = byte[1] * byte[3] / 255;
    //        byte[2] = byte[2] * byte[3] / 255;
    //    }
    //}
}

void GDICanvas::CreateBitmap(uint32_t width, uint32_t height, HBITMAP * bm, void ** data)
{
    BITMAPINFOHEADER hdr = {0};
    hdr.biSize = sizeof(BITMAPINFOHEADER);
    hdr.biWidth = width;
    hdr.biHeight = -(LONG)height;  // minus means top-down bitmap
    hdr.biPlanes = 1;
    hdr.biBitCount = 32;
    hdr.biCompression = BI_RGB;  // no compression
    hdr.biSizeImage = 0;
    hdr.biXPelsPerMeter = 1;
    hdr.biYPelsPerMeter = 1;
    hdr.biClrUsed = 0;
    hdr.biClrImportant = 0;

    *bm = CreateDIBSection(NULL, reinterpret_cast<BITMAPINFO*>(&hdr),
        0, data, NULL, 0);
}

HDC GDICanvas::CreateHDC()
{
    HDC dc = CreateCompatibleDC(NULL);

    BOOL res = ::SetGraphicsMode(dc, GM_ADVANCED);
    // Enables dithering.
    res = ::SetStretchBltMode(dc, HALFTONE);
    // As per SetStretchBltMode() documentation, SetBrushOrgEx() must be called
    // right after.
    res = ::SetBrushOrgEx(dc, 0, 0, NULL);

    // Sets up default orientation.
    res = ::SetArcDirection(dc, AD_CLOCKWISE);
    // Sets up default colors.
    res = ::SetBkColor(dc, RGB(255, 255, 255));
    res = ::SetTextColor(dc, RGB(0, 0, 0));
    res = ::SetDCBrushColor(dc, RGB(255, 255, 255));
    res = ::SetDCPenColor(dc, RGB(0, 0, 0));
    // Sets up default transparency.
    res = ::SetBkMode(dc, OPAQUE);
    res = ::SetROP2(dc, R2_COPYPEN);

    return dc;
}

void GDICanvas::FreeDestination()
{
    if(dest_)
    {
        if(dest_old_obj_)
        {
            ::SelectObject(dest_, dest_old_obj_);
            dest_old_obj_ = 0;
        }
        DeleteDC(dest_);
    }
    if(dest_bitmap_)
    {
        ::DeleteObject(dest_bitmap_);
        dest_bitmap_ = 0;
    }
    dest_width_ = 0;
    dest_height_ = 0;
}

void GDICanvas::FreeSource()
{
    if(src_)
    {
        if(src_old_obj_)
        {
            ::SelectObject(src_, src_old_obj_);
            src_old_obj_ = 0;
        }
        DeleteDC(src_);
    }
    if(src_bitmap_)
    {
        ::DeleteObject(src_bitmap_);
        src_bitmap_ = 0;
    }

    src_data_ = 0;
    src_width_ = 0;
    src_height_ = 0;
}


}


}

}