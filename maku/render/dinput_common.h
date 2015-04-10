#ifndef MAKU_RENDER_DINPUT_COMMON_H_
#define MAKU_RENDER_DINPUT_COMMON_H_

#include <ncore/ncore.h>
#define  DIRECTINPUT_VERSION 0x0800
#include "dinput/dinput.h"

namespace maku
{

namespace render
{
namespace dinputhooker
{
    //DIRECTINPUT
static const size_t kIdxCreateDevice = 0x03;

//DIRECTINPUTDEVICE8
static const size_t kIdxRelease = 2;
static const size_t kIdxSetCooperativeLevel = 0x0d;
static const size_t kIdxAcquire = 7;
static const size_t kIdxUnacquire = 8;
static const size_t kIdxGetDeviceState = 9;
static const size_t kIdxGetDeviceData = 0xa;

typedef HRESULT (WINAPI *DirectInput8Create_T)(HINSTANCE hinst, 
    DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter);

typedef HRESULT (WINAPI *CreateDevice_T)(IDirectInput8 * self, REFGUID iid, 
    LPDIRECTINPUTDEVICE8 * device, LPUNKNOWN unknown);

typedef HRESULT (WINAPI *SetCooperativeLevel_T)(LPDIRECTINPUTDEVICE8 self, 
    HWND hwnd, DWORD flag);

typedef HRESULT (WINAPI *Acquire_T)(LPDIRECTINPUTDEVICE8 self);

typedef HRESULT (WINAPI * Unacquire_T)(LPDIRECTINPUTDEVICE8 self);

typedef ULONG (WINAPI * Release_T)(IUnknown * self);

typedef HRESULT (WINAPI * GetDeviceState_T)(LPDIRECTINPUTDEVICE8 self, 
                         DWORD length, LPVOID buf);


typedef HRESULT(WINAPI * GetDeviceData_T)(LPDIRECTINPUTDEVICE8 self, 
    DWORD,LPDIDEVICEOBJECTDATA,LPDWORD,DWORD);

}


}


}
#endif