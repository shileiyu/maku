#ifndef MAKU_COMMON_RENDER_MSG_DEF_H_
#define MAKU_COMMON_RENDER_MSG_DEF_H_

#include <stdint.h>

#pragma pack(push)
#pragma pack(1)

namespace maku
{

static const char * kPipeName = "\\\\.\\pipe\\maku_render_pipe_";

enum MsgType
{
    //DLL->EXE发的消息
    kHotKey = 0 , //热键
    kMouseEvent,
    kKeyEvent,
    kTextEvent,

    //EXE->DLL
    kCursorChangedEvent,
    kStatusChangedEvent,
    kPaintEvent,
    kMsgCount,
};

//管道消息结构 DLL->EXE
struct PipeMsgHeader
{
    uint32_t type;    //主消息类别 MsgType
    PipeMsgHeader()
    {
        type = 0;
    }
};

struct PipeMouseEvent
{
	uint32_t type;
	uint32_t button;
	uint32_t state;
    int16_t x;
    int16_t y;
    int16_t dx;
    int16_t dy;    
};

struct PipeKeyEvent
{
	uint32_t type;
	uint32_t value;
};

struct PipeTextEvent
{
    wchar_t text;
};

struct PipeStatusChangedEvent
{
    uint32_t show : 1;          //控制是否绘制UI
    uint32_t shield : 1;        //控制鼠标键盘是否路由给游戏中(popup-window时shield=0)
};

//仅在光标样式改变后才发送该事件
struct PipeCursorChangedEvent
{
    int32_t xhotspot;
    int32_t yhotspot;
    uint32_t pixel[32][32];//光标是32 * 32
};

struct PipePaintEvent
{
    uint16_t left;    //绘图脏矩形参数(需要重绘)
    uint16_t top;     //绘图脏矩形参数(需要重绘)
    uint16_t width;   //绘图脏矩形参数(需要重绘)
    uint16_t height;  //绘图脏矩形参数(需要重绘)
    uint32_t bits[1]; //占位符 表示后面紧跟着Redraw的数据
};


}
#pragma pack(pop)

#endif