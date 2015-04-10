#ifndef MAKU_RENDER_UTILS_H_
#define MAKU_RENDER_UTILS_H_
#include <ncore/ncore.h>
#include "geometry.h"

namespace maku
{

namespace render
{

//trans RECT to Rect
void TransRect(RECT & rect1, Rect & rect2);
//trans an int number into 4 float numbers
void TransFloat(uint32_t m, Float4 & n);
//get fps in one second
int GetFPS();

template <typename TKey, typename TValue>
class LazyMap
{
public:
    typedef std::pair<TKey, TValue> Pair;
    typedef std::vector<Pair> PairArray;

public:
    LazyMap() : map_(0){}

    TValue & operator[](const TKey & key)
    {
        if(map_ == 0)
            map_ = new std::map<TKey, TValue>;
        return (*map_)[key];
    };

    bool Has(const TKey & key)
    {
        if(map_ == 0)
            return false;

        return map_->find(key) != map_->end();
    };

    void Remove(const TKey & key)
    {
        if(map_ != 0)
        {
            map_->erase(key);
        }
    }

    std::vector<Pair> Items()
    {
        std::vector<Pair> items;
        if(map_ != 0)
        {
            for(auto iter = map_->begin();  iter != map_->end(); ++iter)
            {
                items.push_back(Pair(iter->first, iter->second));
            }
        }
        return items;
    }
private:
    std::map<TKey, TValue> * map_;
};

}

}
#endif