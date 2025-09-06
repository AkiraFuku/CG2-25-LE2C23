#include<Windows.h>
#include<cstdint>
#include<iostream>
#include<string>
#include<filesystem>
#include<fstream>
#include<chrono>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<strsafe.h>
#include<cassert>
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#include<dbghelp.h>
#pragma comment(lib,"Dbghelp.lib")
#include<dxgidebug.h>
#pragma comment(lib,"dxguid.lib")
#include<dxcapi.h>
#pragma comment(lib,"dxcompiler.lib")
#include"Vector4.h"
#include"MassFunction.h"
#include"externals/imgui/imgui.h"
#include"externals/imgui/imgui_impl_dx12.h"
#include"externals/imgui/imgui_impl_win32.h"
#include"externals/DirectXTex/DirectXTex.h"
#include"externals/DirectXTex/d3dx12.h"
#include<vector>
#include<numbers>
#include<sstream>
#include"Audio.h"
#include <memory>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")





namespace DirectEngine {


    /// <summary>
    /// エンジン初期化
    /// </summary>
    void Initialize(const std::wstring& title = L"LE2C_24_フク_アキラ");
    /// <summary>
    /// エンジン終了
    /// </summary>
    void Finalize();
    /// <summary>
    /// エンジン更新
    /// </summary>
    void Update();

}