#pragma once
#include <Windows.h>
#include "wrl.h"
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include "engine/base/WinApp.h"

class Input
{
public:
    template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

public:
    void Initialize(WinApp* winapp);

    void Update();

    bool PushedKeyDown(BYTE keys);
    bool PushedKeyUp(BYTE keys);
    bool TriggerKeyDown(BYTE keys);
    bool TriggerKeyUp(BYTE keys);


private:
    ComPtr<IDirectInputDevice8> keyboard;
    ComPtr<IDirectInputDevice8> mouse;
    BYTE preKey[256] = {};
    BYTE key[256] = {};

    WinApp* winApp_ = nullptr;
};

