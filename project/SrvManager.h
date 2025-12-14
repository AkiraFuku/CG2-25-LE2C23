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

private:
    DXCommon* dxCommon_ = nullptr;
    uint32_t descriptorSize_;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap_;

    uint32_t useIndex =0;
};

