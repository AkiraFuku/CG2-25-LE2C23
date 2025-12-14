#pragma once
#include <wrl.h>
#include<d3d12.h>
class DXCommon;
class SrvManager
{
public:
    //最大テクスチャ数
    static const uint32_t kMaxSRVCount;
    void Initialize(DXCommon* dxCommon);

    uint32_t Allocate();

     /// <summary>
    /// SRVのCPUディスクリプタハンドルを取得
    /// </summary>
    /// <param name="index"></param>
    /// <returns></returns>
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index);
    /// <summary>
    /// SRVのGPUディスクリプタハンドルを取得
    /// </summary>
    /// <param name="index"></param>
    /// <returns></returns>
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index);

private:
    DXCommon* dxCommon_ = nullptr;
    uint32_t descriptorSize_;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_;

    uint32_t useIndex =0;
};

