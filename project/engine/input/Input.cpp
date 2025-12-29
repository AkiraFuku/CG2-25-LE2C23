#include "Input.h"
#include "assert.h"
#include <memory>

// 静的メンバ変数の初期化
Input* Input::instance = nullptr;

Input* Input::GetInstance() {
    if (instance == nullptr) {
        instance = new Input();
    }
    return instance;
}

void Input::Finalize() {
    delete instance;
    instance = nullptr;
}

void Input::Initialize(WinApp* winapp) {
    // ... (既存のInitializeの中身はそのまま) ...
    winApp_ = winapp;
    // DirectInputの初期化
    HRESULT hr;
    Microsoft::WRL::ComPtr<IDirectInput8> directInput = nullptr;
    // ... 以下省略 (元のコードと同じ) ...
    // ... 末尾まで ...
    for (int i = 0; i < XUSER_MAX_COUNT; ++i) {
        deadZoneL_[i] = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
        deadZoneR_[i] = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
        ZeroMemory(&state_[i], sizeof(XINPUT_STATE));
        ZeroMemory(&preState_[i], sizeof(XINPUT_STATE));
    }
    
    // 初期化時にデバイス状態を取得しておく処理があればここに
    // (元のコードにあった keyboard->GetDeviceState 等)
    if(keyboard) keyboard->GetDeviceState(sizeof(preKey), preKey);
    if(mouse) mouse->GetDeviceState(sizeof(DIMOUSESTATE), &preMouseState);
}

// ... 他のメソッド (Update, PushedKeyDownなど) は変更なし ...