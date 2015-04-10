#ifndef DETOURS_EXT_H_
#define DETOURS_EXT_H_

bool IsModuleDetoured(void * module);
void MarkModuleDetoured(void * module);
bool IsVFDetoured(void * i7e, size_t idx);

template<typename T>
bool GetVFAddress(void * i7e, size_t idx, T & t)
{
    void ** obj = (void **)i7e;
    if(obj == 0)
        return 0;

    void ** vft = (void **)*obj;
    if(vft == 0)
        return 0;

    t = (T)vft[idx];
    return t != 0;
}
#endif