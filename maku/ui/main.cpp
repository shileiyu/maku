#include <ncore/ncore.h>
#include "backroom.h"

int CALLBACK WinMain(HINSTANCE hinst,
    HINSTANCE prev,
    LPSTR cmd_line,
    int show_mode)
{
    using namespace maku::ui;
    return Backroom::Get()->Run();
}