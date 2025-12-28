#include "ImGuiManager.h"
#include "SrvManager.h"
#include "WinApp.h"
#include "DXCommon.h"
void ImGuiManager::Initialize( DXCommon* dxCommon, SrvManager* srv) {
    #ifdef USE_IMGUI

    assert(srv);
    srvManager_ = srv;
    assert(dxCommon);
    dxCommon_ = dxCommon;
  
    winApp_ = dxCommon_->GetWinApp();

  

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
#endif // USE_IMGUI

}
void ImGuiManager::Finalize() {
    #ifdef USE_IMGUI

    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
#endif // USE_IMGUI

}
void ImGuiManager::Begin() {
    #ifdef USE_IMGUI

    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
#endif // USE_IMGUI

}
void ImGuiManager::End() {
    #ifdef USE_IMGUI

    ImGui::Render();
#endif // USE_IMGUI

}
void ImGuiManager::Draw() {
    #ifdef USE_IMGUI

    ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList().Get();

    ID3D12DescriptorHeap* ppHeaps[] = { srvManager_->GetDescriptorHeap().Get() };
    commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
#endif // USE_IMGUI

}