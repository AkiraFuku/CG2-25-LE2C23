#pragma once
#include <Windows.h>
#include "wrl.h"
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

class Input
{
public:
    template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

public:
    void Initialize(HINSTANCE hInstance, HWND hwnd);

    void Update();

    bool PushedKeyDown(BYTE keys);
    bool isPushedKeyUp(BYTE keys);
    bool isPushKeyDown(BYTE keys);
    bool isPushKeyUp(BYTE keys);


private:
    ComPtr<IDirectInputDevice8> keyboard;
    BYTE preKey[256] = {};
    BYTE key[256] = {};
};

