#ifndef MAKU_COMMON_MOUSE_EVENT_H_
#define MAKU_COMMON_MOUSE_EVENT_H_

#include <ncore/ncore.h>

namespace maku
{

class MouseEvent
{
public:
    enum MouseEventType
    {
	    kMouseMove = 1,     //鼠标移动的 
	    kMouseDown,         //鼠标按下
	    kMouseUp,           //鼠标抬起
	    kMouseClick,        //鼠标单击
	    kMouseDoubleClick,  //鼠标双击
	    kMouseWheel,        //鼠标滚轮
	    kMouseEnter,        //进入窗口
	    kMouseLeave,        //离开窗口
	    kMouseHover,        //鼠标悬停
    };

    enum MouseButtons
    {
	    kLeft = 0x100000,
	    kRight = 0x200000,
	    kMiddle = 0x400000,
	    kXButton1 = 0x800000,
	    kXButton2 = 0x1000000,
    };
};

}


#endif
