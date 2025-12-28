#include "Input.h"
#include "assert.h"
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
#include <memory>



void Input::Initialize(WinApp* winapp) {
    winApp_ = winapp;
    // DirectInputの初期化
    HRESULT hr;
    Microsoft::WRL::ComPtr<IDirectInput8> directInput = nullptr;
    hr = DirectInput8Create(
        winApp_->GetInstance(),
        DIRECTINPUT_VERSION,
        IID_IDirectInput8,
        (void**)&directInput,
        nullptr
    );
    assert(SUCCEEDED(hr));
    // キーボードデバイスの作成
    keyboard = nullptr;
    hr = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
    assert(SUCCEEDED(hr));
    hr = keyboard->SetDataFormat(&c_dfDIKeyboard);
    assert(SUCCEEDED(hr));
    // キーボードの設定
    hr = keyboard->SetCooperativeLevel(
        winApp_->GetHwnd(),
        DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY
    );
    assert(SUCCEEDED(hr));
    // マウスデバイスの作成
    mouse = nullptr;
    hr = directInput->CreateDevice(GUID_SysMouse, &mouse, NULL);
    assert(SUCCEEDED(hr));
    hr = mouse->SetDataFormat(&c_dfDIMouse);
    assert(SUCCEEDED(hr));
    hr = mouse->SetCooperativeLevel(
        winApp_->GetHwnd(),
        DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY
    );
    assert(SUCCEEDED(hr));



    keyboard->GetDeviceState(sizeof(preKey), preKey);
    mouse->GetDeviceState(sizeof(DIMOUSESTATE), &preMouseState);
}

void Input::Update()
{
    // 例:

    std::memcpy(preKey, key, sizeof(key));
    keyboard->Acquire();

    keyboard->GetDeviceState(sizeof(key), key);


    // --- マウス更新  ---
    // 前回の状態を保存
    preMouseState = mouseState;
    
    // デバイス取得権限の要求
    mouse->Acquire();
    
    // 現在の状態を取得
    mouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState);
}

bool Input::PushedKeyDown(BYTE keys)
{

    if (key[keys] && preKey[keys])
    {
        return true;

    }
    return false;
}

bool Input::PushedKeyUp(BYTE keys)
{
    //キーボードのキーが離した状態かどうかを確認
    //GetAsyncKeyState関数を使用してキーの状態を取得
    if (!key[keys] && !preKey[keys])
    {
        //キーが押されている状態で、前回のキー状態も押されている場合
        //つまり、キーが離された状態ではない
        return true;

    }

    //キーが離されているかどうかを確認
    //低位ビットが1であればキーが離されている
    return false;
}

bool Input::TriggerKeyDown(BYTE keys)
{
    if (key[keys] && !preKey[keys])
    {
        //キーが押されている状態で、前回のキー状態も押されている場合
        //つまり、キーが離された状態ではない
        return true;

    }

    return false;
}

bool Input::TriggerKeyUp(BYTE keys)
{
    //キーボードのキーを離した瞬間かどうかを確認
    if (!key[keys] && preKey[keys])
    {
        //キーが押されている状態で、前回のキー状態も押されている場合
        //つまり、キーが離された状態ではない
        return true;

    }
    return false;
}
bool Input::PushMouseDown(int32_t button) {
    // マウスボタンが押されているか確認
    int btnIndex = static_cast<int>(button);
    return (mouseState.rgbButtons[btnIndex] & 0x80) != 0;
}

bool Input::PushMouseUP(int32_t button) {
    // マウスボタンが押されていないか確認
    int btnIndex = static_cast<int>(button);
    return (mouseState.rgbButtons[btnIndex] & 0x80) == 0;
}

bool Input::TriggerMouseDown(int32_t button) {
    // マウスボタンがトリガーされたか確認（押した瞬間）
    int btnIndex = static_cast<int>(button);
    return ((mouseState.rgbButtons[btnIndex] & 0x80) != 0 &&
            (preMouseState.rgbButtons[btnIndex] & 0x80) == 0);
}

bool Input::TriggerMouseUP(int32_t button) {
    // マウスボタンがトリガーされたか確認（離した瞬間）
    int btnIndex = static_cast<int>(button);
    return ((mouseState.rgbButtons[btnIndex] & 0x80) == 0 &&
            (preMouseState.rgbButtons[btnIndex] & 0x80) != 0);
}

Input::MoveMouse Input::GetMouseMove()
{
    return MoveMouse(
        mouseState.lX,
        mouseState.lY,
        mouseState.lZ // ホイール
    );
}

