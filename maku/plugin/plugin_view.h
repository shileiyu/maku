#ifndef MAKU_PLUGIN_DEMO_H_
#define MAKU_PLUGIN_DEMO_H_

#include <ncore/sys/stop_watch.h>
#include <ui/plugin.h>
#include <nui/base/pixmap.h>
#include <nui/gadget/world.h>

#include "welcome.h"
#include "ladder.h"

namespace maku
{
namespace ui
{

class PluginView : public Watcher, 
                   public nui::WorldClient
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

    void PopupWelcome();

    void UpdatePixmap();

    void Display();
private:
    Controller * controller_;
    bool show_;
    bool welcome_;
    bool welcoming_;
    nui::Pixmap back_buffer_;
    nui::ScopedWorld world_;
    nui::Rect inval_rect_;
    Welcome logo_;
    ncore::StopWatch watch_;
    bool hotkey_;
    nui::ScopedPicture background_;
    Ladder ladder_;
};

}


}

#endif