#include "utils.h"


namespace maku
{
namespace render
{




void TransRect(RECT & rect1, Rect & rect2)
{
    rect2.SetLTRB(rect1.left, rect1.top, rect1.right, rect1.bottom);
}

void TransFloat(uint32_t m, Float4 & n)
{
    float k = (float)0xff;
    n.x = (float)((m & 0xff0000) >> 16);
    n.y = (float)((m & 0xff00) >> 8);
    n.z = (float)(m & 0xff);
    n.w =(float)((m & 0xff000000) >> 24);
   
    n.x /= k;
    n.y /= k;
    n.z /= k;
    n.w /= k;
}

int GetFPS()
{
    static int fps = 0;
    static int count = 0;
    static float cur_time = 0;
    static float last_time = 0;
    count++;
    cur_time = timeGetTime()*0.001f;
    if (cur_time - last_time >= 1.0f)
    {
        fps = count / (int)(cur_time - last_time);
        last_time = cur_time;
        count = 0;
    }
    return fps;
}

}
}