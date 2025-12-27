#pragma once


class WinApp;
class DXCommon;
class SrvManager;
class ImGuiManager
{
public:
    void Initialize(WinApp* winApp,DXCommon *dxCommon,SrvManager* srv);
    void Finalize();

private:
    WinApp* winApp_ = nullptr;
    DXCommon* dxCommon_ = nullptr;
    SrvManager* srvManager_ = nullptr;
};

