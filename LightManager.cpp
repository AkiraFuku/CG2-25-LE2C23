#include "LightManager.h"
#include "LightManager.h"
#include <cassert>

void LightManager::Initialize(ID3D12Device* device)
{
    // メンバ変数の初期化
    data_.numDirLights = 0;
    data_.numPointLights = 0;
    data_.numSpotLights = 0;

    // 定数バッファの生成
    CreateConstantBuffer(device);
}

void LightManager::CreateConstantBuffer(ID3D12Device* device)
{
    // バッファサイズの計算（256バイトアライメント）
    size_t sizeInBytes = sizeof(LightGroupData);
    sizeInBytes = (sizeInBytes + 0xff) & ~0xff;

    D3D12_HEAP_PROPERTIES heapProps{};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Width = sizeInBytes;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    HRESULT hr = device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&constantBuffer_));
    assert(SUCCEEDED(hr));

    // マッピングしておく
    hr = constantBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedData_));
    assert(SUCCEEDED(hr));
}

void LightManager::TransferBuffer()
{
    if (mappedData_) {
        // CPU側のデータをGPUバッファへコピー
        std::memcpy(mappedData_, &data_, sizeof(LightGroupData));
    }
}

void LightManager::SetGraphicsRootConstantBufferView(ID3D12GraphicsCommandList* commandList, UINT rootParameterIndex)
{
    // GPU仮想アドレスを設定
    commandList->SetGraphicsRootConstantBufferView(rootParameterIndex, constantBuffer_->GetGPUVirtualAddress());
}

// ライト設定の実装例
void LightManager::SetDirectionalLight(int index, const DirectionalLight& light)
{
    if (index >= 0 && index < MAX_DIR_LIGHTS) {
        data_.dirLights[index] = light;
        // 自動的に個数を更新するか、明示的にSetNum...を呼ぶかは設計次第ですが、
        // ここでは「index+1」を個数として最低限確保する簡易ロジックの例
        if (data_.numDirLights < index + 1) {
            data_.numDirLights = index + 1;
        }
    }
}

void LightManager::SetPointLight(int index, const PointLight& light)
{
    if (index >= 0 && index < MAX_POINT_LIGHTS) {
        data_.pointLights[index] = light;
        if (data_.numPointLights < index + 1) {
            data_.numPointLights = index + 1;
        }
    }
}

void LightManager::SetSpotLight(int index, const SpotLight& light)
{
    if (index >= 0 && index < MAX_SPOT_LIGHTS) {
        data_.spotLights[index] = light;
        if (data_.numSpotLights < index + 1) {
            data_.numSpotLights = index + 1;
        }
    }
}