#include "ImGuiManager.h"
#include"imgui.h"
#include"imgui_impl_dx12.h"
#include"imgui_impl_win32.h"

void ImGuiManager::Initialize(WinApp* winApp){
    IMGUI_CHECKVERSION();
    ImGui::GetIO().IniFilename = "externals/imgui/my_imgui_settings.ini";
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(winApp->GetHwnd());
}