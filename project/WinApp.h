#pragma once
#include <Windows.h>
#include<cstdint>
class WinApp
{

public:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

public:
    //初期化
    void Initialize();
    //更新
    void Update();
    //終了
    void Finalize();

        // //ウィンドウのサイズ
   static const int32_t kClientWidth = 1280;
   static const int32_t kClientHeight = 720;

   HWND GetHwnd()const{return hwnd;}
   HINSTANCE GetInstance()const{return wc.hInstance;}
   bool ProcessMessage();
private:
       HWND hwnd=nullptr;
       WNDCLASS wc{};
       //メッセージ
      
};

