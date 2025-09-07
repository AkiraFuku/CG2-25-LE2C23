#include "WinApp.h"
#include "externals/imgui/imgui_impl_win32.cpp"


using namespace DirectEngine;

WinApp* WinApp::GetInstance() {
    static WinApp instance;
    return &instance;
}

LRESULT DirectEngine::WinApp::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
     if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
    {
        return true;
    }

    switch (msg) {
        //
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}



void WinApp::CreateGameWindow(const std::wstring& title, UINT windowStyle, int32_t width, int32_t height) {
  
}

void DirectEngine::WinApp::TerminateGameWindow()
{
}

bool DirectEngine::WinApp::ProcessMessage()
{
    return false;
}

void DirectEngine::WinApp::SetFullscreen(bool Fullscreen)
{
}

bool DirectEngine::WinApp::IsFullscreen() const
{
    return false;
}

void DirectEngine::WinApp::SetSizeChangeMode(SizeChangeMode mode)
{
}

DirectEngine::WinApp::SizeChangeMode DirectEngine::WinApp::GetSizeChangeMode() const
{
    return SizeChangeMode();
}
