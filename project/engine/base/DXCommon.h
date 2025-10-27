#pragma once

#include <d3d12.h>
#include<dxgi1_6.h>
#include <wrl.h>
class DXCommon
{
public:
    void Initialize();

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
};

