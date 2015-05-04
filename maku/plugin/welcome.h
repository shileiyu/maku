#ifndef MAKU_PLUGIN_WELCOME_H_
#define MAKU_PLUGIN_WELCOME_H_

#include <nui/gadget/picture.h>
#include <nui/layout/sizer.h>

namespace maku
{
namespace ui
{

class Welcome
{
public:
    Welcome();

    ~Welcome();

    nui::ScopedGadget GetGadget();

    nui::ScopedPicture Background(){ return background_; }
private:
    nui::ScopedPicture background_;
    nui::ScopedPicture logo_;
    nui::PixelSizer logo_sizer_;
};


}
}
#endif