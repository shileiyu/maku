#ifndef MAKU_PLUGIN_DEMO_H_
#define MAKU_PLUGIN_DEMO_H_

#include <ui/plugin.h>
#include <nui/base/pixmap.h>
#include <nui/gadget/world.h>
#include "welcome.h"

namespace maku
{
namespace ui
{

class PluginView : public Plugin, public nui::GadgetWorldClient
{
public:
    static PluginView * Get();

    PluginView();

    void Load(Controller & controller);

    void Unload(Controller & controller);

    void OnHotKey(Controller & controller) override;

    void OnMouseEvent(Controller & controller, const MouseEvent & e) override;

    void OnKeyEvent(Controller & controller, const KeyEvent & e) override;

    void OnIdle(Controller & controller) override;

    void PenddingRedraw(nui::ScopedWorld world, const nui::Rect & rect) override;

    void SetCursor(nui::ScopedWorld world, nui::CursorStyles cursor) override;
private:
    bool InitNui();

    void FiniNui();
private:
    void PopupWelcome();

    void UpdatePixmap();
private:
    Controller * controller_;
    bool show_;
    bool welcome_;
    nui::Pixmap back_buffer_;
    nui::ScopedWorld world_;
    nui::Rect inval_rect_;
    Welcome logo_;
};

}


}

#endif