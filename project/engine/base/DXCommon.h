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
    //深度バッファ
    Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource_;
    void CreateDepthStencilTextureResource( );

    //各種ディスクプリプターヒープ
    void CreateDescriptorHeaps();
    Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> CreateDescriptorHeap(const  Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE heepType, UINT numDescriptors, bool shaderVisible);
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap_;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap_;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap_;
     uint32_t descriptorSizeSRV_;
     uint32_t descriptorSizeRTV_;
     uint32_t descriptorSizeDSV_;


};

