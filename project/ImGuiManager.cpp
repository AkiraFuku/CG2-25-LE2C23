#include "ImGuiManager.h"
#include"imgui.h"
#include"imgui_impl_dx12.h"
#include"imgui_impl_win32.h"

void ImGuiManager::Initialize(WinApp* winApp,DXCommon *dxCommon){

    assert(winApp);
    winApp_ = winApp;
    assert(dxCommon);
    dxCommon_ = dxCommon;
    IMGUI_CHECKVERSION();
    ImGui::GetIO().IniFilename = "externals/imgui/my_imgui_settings.ini";
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(winApp_->GetHwnd());
     ImGui_ImplDX12_Init(
        dxCommon_->GetDevice().Get(),
        swapChainDesc_.BufferCount,
        rtvDesc_.Format,
        srvHeap_.Get(),
        srvHeap_.Get()->GetCPUDescriptorHandleForHeapStart(),
        srvHeap_.Get()->GetGPUDescriptorHandleForHeapStart()
    );
}