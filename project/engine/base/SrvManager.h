#pragma once
#include <wrl.h>
#include<d3d12.h>
#include<cstdint>
class DXCommon;
class SrvManager
{
public:
    //最大テクスチャ数
    static const uint32_t kMaxSRVCount;
    void Initialize(DXCommon* dxCommon);


    uint32_t AllocateSRV();

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

    void CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT Format, UINT MipLevels);


    void CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structureByteStride);

    void SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex);
    void PreDraw();

    bool IsMax();

     Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() {
         return descriptorHeap_;
     }
private:
    DXCommon* dxCommon_ = nullptr;
    uint32_t descriptorSize_;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_;
    uint32_t useIndex = 0;
};

