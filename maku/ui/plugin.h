#ifndef MAKU_UI_PLUGIN_H_
#define MAKU_UI_PLUGIN_H_

#include <stdint.h>

namespace maku
{
namespace ui
{

struct MouseEvent
{
    uint32_t type;
    uint32_t button;
    uint32_t state;
    int16_t x;
    int16_t y;
    int16_t dx;
    int16_t dy;
};

struct KeyEvent
{
    uint32_t type;
    uint32_t value;
};

struct RedrawEvent
{
    void * bits;
    size_t width;
    size_t height;
    size_t pitch;
    size_t subset_left;
    size_t subset_top;
    size_t subset_width;
    size_t subset_height;
};

class Controller
{
public:
    virtual void Show(bool b) = 0;

    virtual void Shield(bool b) = 0;

    virtual void Redraw(const RedrawEvent & e) = 0;

    virtual size_t GetWidth() = 0;

    virtual size_t GetHeight() = 0;
};

class Plugin
{
public:
    virtual void OnHotKey(Controller & controller) = 0;//热键按下

    virtual void OnMouseEvent(Controller & controller, const MouseEvent & e) = 0;

    virtual void OnKeyEvent(Controller & controller, const KeyEvent & e) = 0;

    virtual void OnIdle(Controller & controller) = 0;
};

}

}


#endif