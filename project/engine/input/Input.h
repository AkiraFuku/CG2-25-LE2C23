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
    // マウスのボタン定義
    enum class MouseButton {
        Left = 0,
        Right = 1,
        Middle = 2,
    };
public:
    void Initialize(WinApp* winapp);

    void Update();
    // --- キーボード用 ---
    bool PushedKeyDown(BYTE keys);
    bool PushedKeyUp(BYTE keys);
    bool TriggerKeyDown(BYTE keys);
    bool TriggerKeyUp(BYTE keys);
    
    // --- マウス用 ---
    /// <summary>
    /// マウスボタンが押されているか
    /// </summary>
    bool PushMouseDown(MouseButton button);
    /// <summary>
    /// マウスボタンが押されないか
    /// </summary>
    bool PushMouseUP(MouseButton button);

    /// <summary>
    /// マウスボタンがトリガーされたか（押した瞬間）
    /// </summary>
    bool TriggerMouseDown(MouseButton button);
    /// <summary>
    /// マウスボタンがトリガーされたか（離した瞬間）
    /// </summary>
    bool TriggerMouseUP(MouseButton button);

    /// <summary>
    /// マウスの移動量を取得
    /// </summary>
    long GetMouseMoveX();
    long GetMouseMoveY();
    long GetMouseMoveZ(); // ホイール

private:
    ComPtr<IDirectInputDevice8> keyboard;
    ComPtr<IDirectInputDevice8> mouse;
    BYTE preKey[256] = {};
    BYTE key[256] = {};


    // --- マウス用変数 ---
    // マウスの状態を格納する構造体
    DIMOUSESTATE mouseState = {};
    DIMOUSESTATE preMouseState = {};

    WinApp* winApp_ = nullptr;
};

