#include "ladder.h"

namespace maku
{

namespace ui
{

void Ladder::Reset(int start, int end, unsigned int uniform_time, unsigned int rest_time)
{
    start_ = start;
    end_ = end;
    uniform_time_ = uniform_time;
    rest_time_ = rest_time;
    gradient_ = static_cast<float>((end - start)) / uniform_time;
}

int Ladder::GetValue(unsigned int cur_time)
{
    if (cur_time < uniform_time_)
    {//start_->end_
        return static_cast<int>(gradient_* cur_time)  + start_;
    }
    else if (cur_time < uniform_time_ + rest_time_)
    {//rest
        return end_;
    }
    else if (cur_time < uniform_time_ * 2 + rest_time_)
    {//end_->start_
        return end_ - static_cast<int>(gradient_* (cur_time - rest_time_ - uniform_time_));
    }
    else
    {
        return start_;
    }
}



}

}