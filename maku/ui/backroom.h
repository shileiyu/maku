#ifndef MAKU_UI_BACKROOM_H_
#define MAKU_UI_BACKROOM_H_


namespace maku
{
namespace ui
{

enum ErrorCode
{
    kErrorSuccess = 0,
    kGeneralFailure = -1,
    kInvalidParams = -2,
    kPipeError = -3,
};

class Backroom
{
public:
    static Backroom * Get();
    
    Backroom();

    ~Backroom();

    ErrorCode Run();
private:
    bool show_;

};
}
}

#endif