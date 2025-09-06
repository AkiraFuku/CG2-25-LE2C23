#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>

namespace DirectEngine {
    /// <summary>
    /// ウィンドウズアプリケーションクラス
    /// </summary>
    class WinApp {

    public:
        //ウィンドウサイズ
        static const int kWindowWidth = 1280;
        static const int kWindowHeight = 720;
        //ウィンドウクラス名
        static const wchar_t kWindowClassName[];

        enum class SizeChangeMode {
            kNone, //サイズ変更なし
            kNormal, //変更可能
            kFixedAspect, //アスペクト比固定

        };

    public://静的メンバ関数

        /// <summary>
        /// インスタンスの取得
        /// </summary>
        /// <returns>インスタンス</returns>
        static WinApp* GetInstance();

        /// <summary>
        /// ウィンドウプロシージャ
        /// </summary>
        /// <param name="hwnd">ウィンドウハンドル</param>
        /// <param name="msg">メッセージ番号</param>
        /// <param name="wparam">メッセージ情報１</param>
        /// <param name="lparam">メッセージ情報２</param>
        /// <returns>正否</returns>
        static LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    public://メンバ関数

        /// <summary>
        /// ウィンドウの作成
        /// </summary> 
        /// <param name="title">タイトルバー</param>
        /// <param name="windowStyle">ウィンドウの初期スタイル</param>
        /// <param name="width">ウィンドウの幅</param>
        /// <param name="height">ウィンドウの高さ</param>
        /// </summary>
        void CreateGameWindow(const std::wstring& title = L"DirectX", UINT windowStyle = WS_OVERLAPPEDWINDOW, int32_t width, int32_t height);

        /// <summary>
        /// ゲームウィンドウの破棄
        /// </summary>
        void TerminateGameWindow();

        /// <summary>
        /// メッセージ処理
        /// </summary>
        /// <returns>終了かどうか</returns>
        bool ProcessMessage();

        /// <summary>
        /// ウィンドウハンドルの取得
        /// </summary>
        HWND GetHwnd() { return hwnd_;}

        HINSTANCE GetHInstance() {return wc_.hInstance;}

        /// <summary>
        /// フルスクリーンの設定
        /// </summary>
        ///<returns></returns>
        void SetFullscreen(bool Fullscreen);
        /// <summary>
        /// フルスクリーンかどうかの取得
        /// </summary>
        /// <returns></returns>
        bool IsFullscreen()const;

        /// <summary>
        /// サイズ変更モードの設定
        /// </summary>
        /// <returns></returns>
        void SetSizeChangeMode(SizeChangeMode mode);

        /// <summary>
        /// サイズ変更モードの取得
        /// </summary>
        /// <returns></returns>
        SizeChangeMode GetSizeChangeMode()const;




    private://メンバ関数
        WinApp() = default;
        ~WinApp() = default;
        WinApp(const WinApp&) = delete;
        WinApp& operator=(const WinApp&) = delete;
    private://メンバ変数
      
        HWND hwnd_ = nullptr; //ウィンドウハンドル
        WNDCLASS wc_ = {}; //ウィンドウクラス
        UINT windowStyle_ ; //ウィンドウスタイル
        bool isFullscreen_ = false; //フルスクリーンかどうか
        RECT windowRect_ = {}; //ウィンドウのサイズ
        SizeChangeMode sizeChangeMode_ = SizeChangeMode::kNone; //サイズ変更モード
        float aspectRatio_ ; //アスペクト比
    };
}


