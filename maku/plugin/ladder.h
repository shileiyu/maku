#ifndef MAKU_PLUGIN_LADDER_H_
#define MAKU_PLUGIN_LADDER_H_

#include <stdint.h>

namespace maku
{


namespace ui
{

class Ladder
{
public:
    void Reset(int start, int end, unsigned int uniform_time, unsigned int rest_time);

    int GetValue(unsigned int cur_time);
private:
    int start_;
    int end_;
    unsigned int uniform_time_;
    unsigned int rest_time_;
    float gradient_;
};

}

}

#endif