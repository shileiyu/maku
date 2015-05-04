#include "welcome.h"

namespace maku
{
namespace ui
{

Welcome::Welcome()
{
    using namespace nui;
    background_ = new Picture;
    logo_ = new Picture;

    background_->Set(0xff000000);
    logo_->Set(0xffffffff);
    logo_sizer_.left(10).right(10).top(5).bottom(5);
    logo_sizer_.Attach(logo_);
    //background_->SetLayout(&logo_sizer_);
}

Welcome::~Welcome()
{
    //lazy
}

nui::ScopedGadget Welcome::GetGadget()
{
    return background_;
}

}
}