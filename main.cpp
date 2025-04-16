#include <Windows.h>
#include<cstdint>

    LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
        switch (msg) {
            //
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        
        }
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
//winmain
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int){
    //ウィンドウのサイズ
    const int32_t kClientWidth = 1280;
    const int32_t kClientHeight = 720;

    RECT wrc = { 0, 0, kClientWidth, kClientHeight };

    AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, FALSE);

    WNDCLASS wc={};
    //ウィンドウプロシージャ
    wc.lpfnWndProc = WindowProc;
    //ウィンドウクラスの名前
    wc.lpszClassName=L"CG2WindowClass";
    //インスタンスハンドル
    wc.hInstance = GetModuleHandle(nullptr);
    //カーソル
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    //ウィンドウクラスの登録
    RegisterClass(&wc);
	// Output
	OutputDebugStringA("Hello,DirectX!\n");
      HWND hwnd = CreateWindow(
            wc.lpszClassName,//クラス名
            L"CG2",
         WS_OVERLAPPEDWINDOW,
         CW_USEDEFAULT,
         CW_USEDEFAULT,
         wrc.right - wrc.left,
         wrc.bottom - wrc.top,
         nullptr,
         nullptr,
         wc.hInstance,
         nullptr
     );
    MSG msg{};
    while (msg.message != WM_QUIT) {
        //メッセージがある限りGetMessageを呼び出す
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else
        {
            //ウィンドウを表示
            ShowWindow(hwnd, SW_SHOW);
        }
        //メッセージがない場合は描画処理を行う
    }
 

    
	return 0;
}