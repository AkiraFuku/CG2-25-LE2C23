#include "Input.h"
#include "assert.h"
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
#include <memory>



void Input::Initialize(HINSTANCE hInstance,HWND hwnd){
     // DirectInputの初期化
       HRESULT hr;
            Microsoft::WRL::ComPtr<IDirectInput8> directInput = nullptr;
             hr = DirectInput8Create(
                hInstance,
                DIRECTINPUT_VERSION,
                IID_IDirectInput8,
                (void**)&directInput,
                nullptr
            );
            assert(SUCCEEDED(hr));
            // キーボードデバイスの作成
             keyboard= nullptr;
            hr = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
            assert(SUCCEEDED(hr));
   hr = keyboard->SetDataFormat(&c_dfDIKeyboard);
            assert(SUCCEEDED(hr));
            // キーボードの設定
            hr= keyboard->SetCooperativeLevel(
                hwnd,
                DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY
            );
            assert(SUCCEEDED(hr));
           // マウスデバイスの作成
            Microsoft::WRL::ComPtr<IDirectInputDevice8> mouse = nullptr;
            hr = directInput->CreateDevice(GUID_SysMouse, &mouse, NULL);
            assert(SUCCEEDED(hr));
            hr=mouse->SetDataFormat(&c_dfDIMouse);
            assert(SUCCEEDED(hr));
            hr = mouse->SetCooperativeLevel(
                hwnd,
                DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY
            );
            assert(SUCCEEDED(hr));

            
              BYTE preKey[256] = {};
              keyboard->GetDeviceState(sizeof(preKey), preKey);
            
}

void Input::Update()
{
       // 例:
           keyboard->Acquire();
           BYTE key[256] = {};
         keyboard->GetDeviceState(sizeof(key), key);


            std::memcpy(preKey, key, sizeof(key));
}
