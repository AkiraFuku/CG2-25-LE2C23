#include "ImGuiManager.h"
#include"imgui.h"
#include"imgui_impl_dx12.h"
#include"imgui_impl_win32.h"
#include "SrvManager.h"
#include "WinApp.h"
#include "DXCommon.h"
void ImGuiManager::Initialize(WinApp* winApp, DXCommon* dxCommon, SrvManager* srv) {

    assert(winApp);
    winApp_ = winApp;
    assert(dxCommon);
    dxCommon_ = dxCommon;
    assert(srv);
    srvManager_ = srv;
    
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = "externals/imgui/my_imgui_settings.ini";
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(winApp_->GetHwnd());
    uint32_t fontSrvIndex =
        srvManager_->AllocateSRV(); // フォント用SRVを確保

   // descriptorHeap_=;
    ImGui_ImplDX12_Init(
        dxCommon_->GetDevice().Get(),
        static_cast<int>(dxCommon_->GetSwapChainBufferCount()),
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
       srvManager_->GetDescriptorHeap().Get(),
        srvManager_->GetCPUDescriptorHandle(fontSrvIndex),
        srvManager_->GetGPUDescriptorHandle(fontSrvIndex)

    );
}
void ImGuiManager::Finalize() {
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}
void ImGuiManager::Begin() {
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}
void ImGuiManager::End() {
    ImGui::Render();
}
void ImGuiManager::Draw() {

    ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList().Get();

    ID3D12DescriptorHeap* ppHeaps[] = { srvManager_->GetDescriptorHeap().Get() };
    commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
}