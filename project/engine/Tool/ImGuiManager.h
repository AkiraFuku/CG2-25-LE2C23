#pragma once
#include <wrl.h>

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

    void Initialize( DXCommon* dxCommon, SrvManager* srv);
    void Finalize();
    void Begin();
    void End();
    void Draw();
private:
    WinApp* winApp_ = nullptr;
    DXCommon* dxCommon_ = nullptr;
    SrvManager* srvManager_ = nullptr;
   
};

