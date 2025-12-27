#pragma once
#include <wrl.h>

class WinApp;
class DXCommon;
class SrvManager;
class ImGuiManager
{
public:
    void Initialize(WinApp* winApp, DXCommon* dxCommon, SrvManager* srv);
    void Finalize();
    void Begin();
    void End();
    void Draw();
private:
    WinApp* winApp_ = nullptr;
    DXCommon* dxCommon_ = nullptr;
    SrvManager* srvManager_ = nullptr;
   
};

