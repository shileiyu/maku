#include "demo.h"
#include <nui/base/pixpainter.h>
#include <ncore/base/buffer.h>

namespace maku
{

namespace ui
{
static const uint32_t kAnimationTime = 1000;
static const nui::Color kHotKeyColor = nui::Color(192, 0, 0, 0);
static const nui::Color kPopupColor = nui::Color(0, 0, 0, 0);

PluginView * PluginView::Get()
{
    static PluginView * view = nullptr;
    if (!view)
        view = new PluginView;

    return view;
}

PluginView::PluginView()
:controller_(nullptr), show_(false), welcome_(false), welcoming_(false), hotkey_(false)
{
    ;
}

void PluginView::Load(Controller & controller)
{
    controller_ = &controller;
    //载入 
    //初始化
    //skia initialize
    //SkGraphics::Init();
    InitNui();
    //准备弹出欢迎消息
    welcome_ = true;
}

void PluginView::Unload(Controller & controller)
{
    //OnlyLazy
    FiniNui();
    //skia uninitialize
    //SkGraphics::Term();
}

bool PluginView::InitNui()
{
    using namespace nui;
    auto width = controller_->GetWidth();
    auto height = controller_->GetHeight();
    auto pitch = width * 4;//argb
    back_buffer_ = Pixmap::Alloc(width, height);
    world_ = new World(this);
    world_->SetSize(Size::Make(width, height));
    background_->SetSize(Size::Make(width, height));
    world_->AddChild(background_);

    //logo 大小固定 水平居中
    auto logo_gadget = logo_.GetGadget();
    logo_gadget->SetSize(Size::Make(300, 40));
    logo_gadget->SetLoc(Point::Make((width - 300) / 2, 0));
    logo_gadget->SetVisible(false);
    world_->AddChild(logo_gadget);
    //排版
    world_->Layout();
    welcome_ = true;
    return true;
}

void PluginView::FiniNui()
{
    //lazy
}

void PluginView::PopupWelcome()
{
    using namespace nui;
    
    if (welcome_)
    {
        auto gadget = logo_.GetGadget();
        gadget->SetTop(-gadget->GetHeight());
        gadget->SetVisible(true);
        welcome_ = false;
        welcoming_ = true;
        watch_.Start();
        background_->SetBackground(hotkey_ ? kHotKeyColor : kPopupColor);
        controller_->Display(welcoming_ || hotkey_, hotkey_);
    }
    if (welcoming_)
    {
        auto gadget = logo_.GetGadget();
        float interval = static_cast<float>(watch_.ElapsedTime());
        auto height = gadget->GetHeight();
        float distance = interval / kAnimationTime * height;
        int idistance = static_cast<int>(distance);
        if (idistance <= height)
        {//welcome下降
            gadget->SetTop(-height + idistance);
        }
        else if ( (idistance -= height) <= height )
        {//welcome上升
            gadget->SetTop(-idistance);
        }
        else
        {//welcome结束
            welcoming_ = false;
            gadget->SetTop(-height);
            gadget->SetVisible(false);
            watch_.Stop();
            controller_->Display(welcoming_ || hotkey_, hotkey_);
        }
    }
}

void PluginView::PenddingRedraw(nui::ScopedWorld world, const nui::Rect & rect)
{
    inval_rect_.Union(rect);
}

void PluginView::SetCursor(nui::ScopedWorld world, nui::CursorStyles cursor)
{
    ;
}

void PluginView::OnHotKey(Controller & controller)
{
    hotkey_ = !hotkey_;
    controller_->Display(welcoming_ || hotkey_, hotkey_);
}

void PluginView::OnMouseEvent(Controller & controller, const MouseEvent & e)
{
    ;
}

void PluginView::OnKeyEvent(Controller & controller, const KeyEvent & e)
{
    ;
}

void PluginView::OnIdle(Controller & controller)
{

    PopupWelcome();
    world_->Blink();
    UpdatePixmap();

}

void PluginView::UpdatePixmap()
{
    using namespace nui;
    if (inval_rect_.isEmpty())
        return;
    PixPainter painter(back_buffer_);
    world_->Draw(painter, inval_rect_);
    //发送到render中去
    PixStorage::Outline ol;
    Rect subset;
    back_buffer_.Lock(ol, subset);

    RedrawEvent pe;
    pe.bits = ol.bits;
    pe.width = ol.width;
    pe.height = ol.height;
    pe.pitch = ol.pitch;

    pe.subset_left = inval_rect_.left();
    pe.subset_top = inval_rect_.top();
    pe.subset_width = inval_rect_.width();
    pe.subset_height = inval_rect_.height();

    controller_->Redraw(pe);

    back_buffer_.Unlock();

    inval_rect_.SetEmpty();
}


}

}