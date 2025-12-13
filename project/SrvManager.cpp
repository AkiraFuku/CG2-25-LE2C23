#include "SrvManager.h"
#include "DXCommon.h"
const uint32_t SrvManager::kMaxSRVCount = 512;

void SrvManager::Initialize(DXCommon* dxCommon) {
    dxCommon_ = dxCommon;
    descriptorHeap_ = dxCommon_->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kMaxSRVCount, true);
    descriptorSize_ = dxCommon_->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}
uint32_t SrvManager::Allocate() {
    assert(useIndex > kMaxSRVCount);
    int index = useIndex;
    useIndex++;
    return index;
}
D3D12_CPU_DESCRIPTOR_HANDLE SrvManager::GetCPUDescriptorHandle(uint32_t index)
{
    D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap_->GetCPUDescriptorHandleForHeapStart();
    handleCPU.ptr += (descriptorSize_ * index);
    return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE SrvManager::GetGPUDescriptorHandle(uint32_t index)
{
    D3D12_GPU_DESCRIPTOR_HANDLE handleGPU= descriptorHeap_->GetGPUDescriptorHandleForHeapStart();
    handleGPU.ptr+=(descriptorSize_*index);
    return handleGPU;
}
