#include "plugin_view.h"
#include <nui/base/pixpainter.h>
#include <ncore/base/buffer.h>
#include <nui/gadget/picture.h>
#include <nui/winnt/workspace.h>
#include <nui/winnt/utils.h>

namespace maku
{

namespace ui
{
static const uint32_t kAnimationTime = 2000;
static const nui::Color kHotKeyColor = 0xc0000000;
static const nui::Color kPopupColor = 0;

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
    controller_->AddWatcher(this);
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
    controller_->RemoveWatcher(this);
}

bool PluginView::InitNui()
{
    using namespace nui;
    auto width = controller_->GetWidth();
    auto height = controller_->GetHeight();
    back_buffer_ = Pixmap::Alloc(width, height);
    world_ = new World(this);
    world_->SetSize(Size::Make(width, height));
    background_ = new Picture;
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
        gadget->SetVisible(true);
        welcome_ = false;
        welcoming_ = true;
        ladder_.Reset(-gadget->GetHeight(), 0, kAnimationTime, kAnimationTime);
        watch_.Start();
        Display();
    }

    if (welcoming_)
    {
        auto gadget = logo_.GetGadget();
        auto time = watch_.ElapsedTime();
        int top = ladder_.GetValue(time);
        gadget->SetTop(top);
        if (time >= kAnimationTime * 3)
        {//动画终止 welcome结束
            welcoming_ = false;
            gadget->SetVisible(false);
            watch_.Stop();
            Display();
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
    Display();
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

void PluginView::Display()
{
    background_->Set(hotkey_ ? kHotKeyColor : kPopupColor);
    controller_->Display(welcoming_ || hotkey_, hotkey_);
}

void PluginView::UpdatePixmap()
{
    using namespace nui;
    if (inval_rect_.isEmpty())
        return;
    inval_rect_.Intersect(Rect::Make(0, 0, world_->GetWidth(), world_->GetHeight()));
    if (inval_rect_.isEmpty())
        return;
    PixPainter painter(back_buffer_);
    Rect tmp(inval_rect_);
    inval_rect_.SetEmpty();

    world_->Draw(painter, tmp);
    //发送到render中去
    PixStorage::Outline ol;
    Rect subset;
    back_buffer_.Lock(ol, subset);

    RedrawEvent pe;
    pe.bits = ol.bits;
    pe.width = ol.width;
    pe.height = ol.height;

    pe.subset_left = tmp.left();
    pe.subset_top = tmp.top();
    pe.subset_width = tmp.width();
    pe.subset_height = tmp.height();

    controller_->Redraw(pe);

    back_buffer_.Unlock();
}


}

}