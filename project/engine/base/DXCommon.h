#pragma once

#include <d3d12.h>
#include<dxgi1_6.h>
#include <wrl.h>
#include "WinApp.h"
class DXCommon
{
public:
    void Initialize( WinApp* winApp);

private:
    void CreateDevice();
    //D3D12デバイス
    Microsoft::WRL::ComPtr<ID3D12Device> device_;
    //DXGIファクトリー
    Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;

    //コマンドキュー
    void CreateCommand();
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
    //コマンドアロケーター
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
    //コマンドリスト  
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;
    //スワップチェーン
    void CreateSwapChain();
    Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;

    WinApp* winApp_ = nullptr;

};

