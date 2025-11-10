#pragma once

#include <d3d12.h>
#include<dxgi1_6.h>
#include <wrl.h>
#include "WinApp.h"
#include <array> // ← 追加

class DXCommon
{
public:
    void Initialize(WinApp* winApp);

    /// <summary>
    /// SRVのCPUディスクリプタハンドルを取得
    /// </summary>
    /// <param name="index"></param>
    /// <returns></returns>
    D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);
    /// <summary>
    /// SRVのGPUディスクリプタハンドルを取得
    /// </summary>
    /// <param name="index"></param>
    /// <returns></returns>
    D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);

private:
    HRESULT hr_;

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
    void CreateDepthStencilTextureResource();

    //各種ディスクプリプターヒープ
    void CreateDescriptorHeaps();
    Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> CreateDescriptorHeap(const  Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE heepType, UINT numDescriptors, bool shaderVisible);
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap_;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap_;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap_;
    uint32_t descriptorSizeSRV_;
    uint32_t descriptorSizeRTV_;
    uint32_t descriptorSizeDSV_;
    //レンダーターゲットビュー
    void CreateRenderTargetView();
    std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swapChainResources_;
    //ディスクリプタ２つ用意
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[2];
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index);
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index);
    //深度ステンシルビュー
    void CreateDepthStencilView();
    //フェンス
    void CreateFence();
    Microsoft::WRL::ComPtr<ID3D12Fence> fence_ = nullptr;
    //ビューポート矩形
    void CreateViewport();
    D3D12_VIEWPORT viewport_{};
    //シザー矩形
    D3D12_RECT scissorRect_{};
    void CreateScissorRect();
};

