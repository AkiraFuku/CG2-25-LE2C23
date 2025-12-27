#pragma once
#include "WinApp.h"
#include "DXCommon.h"
class ImGuiManager
{
public:
    void Initialize(WinApp* winApp,DXCommon *dxCommon);

private:
    WinApp* winApp_ = nullptr;
    DXCommon* dxCommon_ = nullptr;
};

