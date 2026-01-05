#pragma once
#include <wrl.h>
#include <memory>
#ifdef USE_IMGUI
#include"imgui.h"
#include"imgui_impl_dx12.h"
#include"imgui_impl_win32.h"

#endif // USE_IMGUI

class WinApp;
class DXCommon;
class SrvManager;
class ImGuiManager
{
public:

    // インスタンス取得
    static ImGuiManager* GetInstance();
    // スマートポインタ用
    static std::unique_ptr<ImGuiManager> instance;
     friend struct std::default_delete<ImGuiManager>;
    void Initialize( DXCommon* dxCommon, SrvManager* srv);
    void Finalize();
    void Begin();
    void End();
    void Draw();
private:
    // シングルトン化のためコンストラクタ類をprivateにする
    ImGuiManager() = default;
    ~ImGuiManager() = default;
    ImGuiManager(const ImGuiManager&) = delete;
    ImGuiManager& operator=(const ImGuiManager&) = delete;

    WinApp* winApp_ = nullptr;
    DXCommon* dxCommon_ = nullptr;
    SrvManager* srvManager_ = nullptr;   
};

