#include "SrvManager.h"
#include "DXCommon.h"
const uint32_t SrvManager::kMaxSRVCount = 512;

void SrvManager::Initialize(DXCommon* dxCommon) {
    dxCommon_ = dxCommon;
}
void SrvManager::PreDraw(){
    ID3D12DescriptorHeap* descritptorHeaps[]={descriptorHeap_.Get()};
    dxCommon_->GetCommandList()->SetDescriptorHeaps(1,descritptorHeaps);
}
uint32_t SrvManager::Allocate() {
    assert(useIndex > kMaxSRVCount);
    int index = useIndex;
    useIndex++;
    return index;
}
D3D12_CPU_DESCRIPTOR_HANDLE SrvManager::GetCPUDescriptorHandle(uint32_t index) {
    D3D12_CPU_DESCRIPTOR_HANDLE handleCPU =descriptorHeap_->GetCPUDescriptorHandleForHeapStart();
    handleCPU.ptr+=(descriptorSize_*index);
    return handleCPU;
}
D3D12_GPU_DESCRIPTOR_HANDLE SrvManager::GetGPUDescriptorHandle(uint32_t index) {
    D3D12_GPU_DESCRIPTOR_HANDLE handleGPU=descriptorHeap_->GetGPUDescriptorHandleForHeapStart();
    handleGPU.ptr+=(descriptorSize_*index);
    return handleGPU;
}
void SrvManager::CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT Format, UINT MipLevels){
     D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = Format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
    srvDesc.Texture2D.MipLevels = UINT( MipLevels);//最初のミップマップ
    // SRV
    dxCommon_->GetDevice()->CreateShaderResourceView(
        pResource,
        &srvDesc,
        GetCPUDescriptorHandle(srvIndex)
    );
}
void SrvManager::CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structureByteStride){
     D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
    srvDesc.Buffer.NumElements = numElements;
    srvDesc.Buffer.StructureByteStride = structureByteStride;
    D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU = GetCPUDescriptorHandle(srvIndex);
    D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU = GetGPUDescriptorHandle(srvIndex);
    dxCommon_->GetDevice()->CreateShaderResourceView(pResource, &srvDesc, instancingSrvHandleCPU);

}
void SrvManager::SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex){
    dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(RootParameterIndex,GetGPUDescriptorHandle(srvIndex));
}
