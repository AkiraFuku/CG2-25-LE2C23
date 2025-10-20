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
             mouse = nullptr;
            hr = directInput->CreateDevice(GUID_SysMouse, &mouse, NULL);
            assert(SUCCEEDED(hr));
            hr=mouse->SetDataFormat(&c_dfDIMouse);
            assert(SUCCEEDED(hr));
            hr = mouse->SetCooperativeLevel(
                hwnd,
                DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY
            );
            assert(SUCCEEDED(hr));

            
          
              keyboard->GetDeviceState(sizeof(preKey), preKey);
            
}

void Input::Update()
{
       // 例:

         std::memcpy(preKey, key, sizeof(key));
           keyboard->Acquire();
       
         keyboard->GetDeviceState(sizeof(key), key);


           
}

bool Input::PushedKeyDown(BYTE keys)
{

    if (key[keys]&&preKey[keys])
    {
        return true;    

    }
    return false;
}

bool Input::PushedKeyUp(BYTE keys)
{
    //キーボードのキーが離した状態かどうかを確認
    //GetAsyncKeyState関数を使用してキーの状態を取得
    if (!key[keys]&&!preKey[keys])
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
    if (key[keys]&&!preKey[keys])
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
    if (!key[keys]&&preKey[keys])
    {
        //キーが押されている状態で、前回のキー状態も押されている場合
        //つまり、キーが離された状態ではない
        return true;

    }
    return false;
}
