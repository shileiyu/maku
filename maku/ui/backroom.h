#ifndef MAKU_UI_BACKROOM_H_
#define MAKU_UI_BACKROOM_H_

#include <ncore/sys/message_loop.h>
#include <ncore/base/buffer.h>
#include <nui/gadget/world.h>
#include <nui/base/pixmap.h>
#include <nui/base/rect.h>

namespace maku
{
namespace ui
{

enum ErrorCode
{
    kErrorSuccess = 0,
    kGeneralFailure = -1,
    kInvalidParams = -2,
    kErrorPipe = -3,
};

class Backroom : public ncore::MessageLoop::Observer,
                 public nui::GadgetWorldClient
{
public:
    static Backroom * Get();
    
    Backroom();

    ~Backroom();

    ErrorCode Run();

protected:
    uint32_t OnIdle(ncore::MessageLoop & loop) override;

    void PenddingRedraw(nui::ScopedWorld world, const nui::Rect & rect) override;

    void SetCursor(nui::ScopedWorld world, nui::CursorStyles cursor) override;
private:
    bool InitView(int width, int height);

    void FiniView();

    void PushPixmap();
private:
    nui::ScopedWorld world_;
    nui::Pixmap back_buffer_;
    nui::Rect inval_rect_;
    ncore::Buffer * raw_buffer_;
};

}
}

#endif