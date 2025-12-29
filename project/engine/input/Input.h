#pragma once
#include <Windows.h>
#include "wrl.h"
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <Xinput.h>
#pragma comment(lib, "xinput.lib")
#include "WinApp.h"
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

class Input
{
public:
    template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
    
    // シングルトン化に伴う追記
    static Input* GetInstance();
    void Finalize();

    // マウスのボタン定義
    enum  MouseButton {
        Left = 0,
        Right = 1,
        Middle = 2,
    };
    struct MoveMouse
    {
        LONG x;
        LONG y;
        LONG z;
    };

    void Initialize(WinApp* winapp);
    void Update();

    // --- キーボード用 ---
    bool PushedKeyDown(BYTE keys);
    bool PushedKeyUp(BYTE keys);
    bool TriggerKeyDown(BYTE keys);
    bool TriggerKeyUp(BYTE keys);

    // --- マウス用 ---
    bool PushMouseDown(int32_t button);
    bool PushMouseUP(int32_t button);
    bool TriggerMouseDown(int32_t button);
    bool TriggerMouseUP(int32_t button);
    MoveMouse GetMouseMove();

    // ゲームパッド用
    bool GetJoyStick(int32_t stickNo, XINPUT_STATE& out) const;
    bool GetPreJoyStick(int32_t stickNo, XINPUT_STATE& out) const;
    void SetDeadZone(int32_t stickNo, int32_t deadZoneL, int32_t deadZoneR);
    size_t GetConnectedStickNum();

    bool PushPadDown(int32_t stickNo, WORD button) const;
    bool PushPadUP(int32_t stickNo, WORD button) const;
    bool TriggerPadDown(int32_t stickNo, WORD button) const;
    bool TriggerPadUP(int32_t stickNo, WORD button) const;

private:
    // シングルトンパターン: コンストラクタ等を隠蔽
    static Input* instance;
    Input() = default;
    ~Input() = default;
    Input(const Input&) = delete;
    Input& operator=(const Input&) = delete;

    ComPtr<IDirectInputDevice8> keyboard;
    ComPtr<IDirectInputDevice8> mouse;
    BYTE preKey[256] = {};
    BYTE key[256] = {};

    // --- マウス用変数 ---
    DIMOUSESTATE mouseState = {};
    DIMOUSESTATE preMouseState = {};
    WinApp* winApp_ = nullptr;

    // --- ゲームパッド用変数 ---
    int deadZoneL_[XUSER_MAX_COUNT];
    int deadZoneR_[XUSER_MAX_COUNT];
    
    XINPUT_STATE state_[XUSER_MAX_COUNT];
    XINPUT_STATE preState_[XUSER_MAX_COUNT];
};