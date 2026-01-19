#include "LightManager.h"
#include "MathFunction.h" // Normalize関数など
#include "DXCommon.h"
// 静的メンバ変数の実体定義
std::unique_ptr<LightManager> LightManager::instance = nullptr;

LightManager* LightManager::GetInstance() {
    if (instance == nullptr) {
        instance.reset(new LightManager());
    }
    return instance.get();
}

void LightManager::Finalize() {
    instance.reset();
}

void LightManager::Initialize() {
  const int kMaxLights = 100;
  CreateStructuredBuffer(sizeof(DirectionalLightData) * kMaxLights, dirLightBuff_);
    CreateStructuredBuffer(sizeof(PointLightData) * kMaxLights, pointLightBuff_);
    CreateStructuredBuffer(sizeof(SpotLightData) * kMaxLights, spotLightBuff_);
    
    CreateConstBuffer(sizeof(LightCounts), countBuff_);
    countBuff_->Map(0, nullptr, reinterpret_cast<void**>(&countData_));
    //// 初期値の設定（全て無効化状態で初期化）
    //for (int i = 0; i < kNumDirectionalLights; ++i) {
    //    lightData_.directionalLights[i].color = { 1.0f, 1.0f, 1.0f, 1.0f };
    //    lightData_.directionalLights[i].direction = { 0.0f, -1.0f, 0.0f };
    //    lightData_.directionalLights[i].intensity = 0.0f;
    //}
    //for (int i = 0; i < kNumPointLights; ++i) {
    //    lightData_.PointLightLights[i].color = { 1.0f, 1.0f, 1.0f, 1.0f };
    //    lightData_.PointLightLights[i].radius = 0.0f;
    //    lightData_.PointLightLights[i].intensity = 0.0f;
    //    lightData_.PointLightLights[i].position = { 0.0f, 0.0f, 0.0f };
    //    lightData_.PointLightLights[i].decay = 0.1f;
    //}
    //for (int i = 0; i < kNumSpotLights; ++i) {
    //    lightData_.SpotLights[i].color = { 1.0f, 1.0f, 1.0f, 1.0f }; // 白
    //    lightData_.SpotLights[i].position = { 2.0f, 1.25f, 0.0f };    // 座標 (Y=2.0)
    //    lightData_.SpotLights[i].distance = 7.0f;
    //    lightData_.SpotLights[i].direction = Normalize({ -1.0f,-1.0f,0.0f });
    //    lightData_.SpotLights[i].intensity = 0.0f;  // 強度
    //    lightData_.SpotLights[i].decay = 2.0f;
    //    lightData_.SpotLights[i].cosAngle = 0.0f;
    //    lightData_.SpotLights[i].cosFalloffStart = 0.0f;
    //}
}

void LightManager::ClearLights() {
    directionalLights_.clear();
    pointLights_.clear();
    spotLights_.clear();
}

void LightManager::AddDirectionalLight(const Vector4& color, const Vector3& direction, float intensity) {

    DirectionalLightData light;
    light.color=color;
    light.direction=direction;
    light.intensity=intensity;
    

    directionalLights_.push_back(light);
}
void  LightManager::AddPointLight(const Vector4& color, const Vector3& position, float intensity, float radius, float decay){
    PointLightData light;
       light.color=color;
       light.position=position;
       light.intensity=intensity;
       light.radius=radius;
       light.decay=decay;
       pointLights_.push_back(light);
}

void LightManager::AddSpotLight(const Vector4& color, const Vector3& position, float intensity, const Vector3& direction, float distance, float decay, float cosAngle, float cosFalloffStart)
{
    SpotLightData light;
    light.color=color;
    light.position=position;
    light.intensity=intensity;
    light.direction=direction;
    light.distance=distance;
    light.decay=decay;
    light.cosAngle=cosAngle;
    light.cosFalloffStart=cosFalloffStart;

}



void LightManager::Update() {

    // 1. カウント情報の更新
    countData_->numDirectional = (int)directionalLights_.size();
    countData_->numPoint = (int)pointLights_.size();
    countData_->numSpot = (int)spotLights_.size();
    if (!directionalLights_.empty()) {
        DirectionalLightData* dst = nullptr;
        dirLightBuff_->Map(0, nullptr, reinterpret_cast<void**>(&dst));
        memcpy(dst, directionalLights_.data(), sizeof(DirectionalLightData) * directionalLights_.size());
        dirLightBuff_->Unmap(0, nullptr);
    }
    if (!pointLights_.empty()) {
        PointLightData* dst = nullptr;
        pointLightBuff_->Map(0, nullptr, reinterpret_cast<void**>(&dst));
        memcpy(dst, pointLights_.data(), sizeof(PointLightData) * pointLights_.size());
        pointLightBuff_->Unmap(0, nullptr);
    }
    if (!spotLights_.empty()) {
        SpotLightData* dst = nullptr;
        spotLightBuff_->Map(0, nullptr, reinterpret_cast<void**>(&dst));
        memcpy(dst, spotLights_.data(), sizeof(SpotLightData) * spotLights_.size());
        spotLightBuff_->Unmap(0, nullptr);
    }
    //// 定数バッファへコピー
    //if (constBufferData_) {
    //    for (int i = 0; i < kNumDirectionalLights; ++i) {
    //        constBufferData_->directionalLights[i] = lightData_.directionalLights[i];
    //    }
    //    for (int i = 0; i < kNumPointLights; ++i) {
    //        constBufferData_->PointLightLights[i] = lightData_.PointLightLights[i];
    //    }
    //    for (int i = 0; i < kNumSpotLights; ++i) {
    //        constBufferData_->SpotLights[i] = lightData_.SpotLights[i];
    //    }
    //}
}

void LightManager::Draw(UINT rootParameterIndex) {
    //if (!constBufferResource_) return;

    //// コマンドリストに定数バッファをセット
    //DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(
    //    rootParameterIndex,
    //    constBufferResource_->GetGPUVirtualAddress()
    //);

    // Dir (Index 3)
     DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootShaderResourceView(rootParameterIndex + 0, dirLightBuff_->GetGPUVirtualAddress());
    // Point (Index 4)
     DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootShaderResourceView(rootParameterIndex + 1, pointLightBuff_->GetGPUVirtualAddress());
    // Spot (Index 5)
     DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootShaderResourceView(rootParameterIndex + 2, spotLightBuff_->GetGPUVirtualAddress());
    // Count (Index 6)
     DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(rootParameterIndex + 3, countBuff_->GetGPUVirtualAddress());
}

//void LightManager::SetDirectionalLight(int index, const Vector4& color, const Vector3& direction, float intensity) {
//    if (index < 0 || index >= kNumDirectionalLights) return;
//
//    lightData_.directionalLights[index].color = color;
//    lightData_.directionalLights[index].direction = direction;
//    lightData_.directionalLights[index].intensity = intensity;
//}

//void LightManager::SetPointLight(int index, const Vector4& color, const Vector3& position, float intensity, float radius, float decay) {
//    if (index < 0 || index >= kNumPointLights) return;
//
//    lightData_.PointLightLights[index].color = color;
//    lightData_.PointLightLights[index].position = position;
//    lightData_.PointLightLights[index].intensity = intensity;
//    lightData_.PointLightLights[index].radius = radius;
//    lightData_.PointLightLights[index].decay = decay;
//}
//
//void LightManager::SetSpotLight(int index, const Vector4& color, const Vector3& position, float intensity, const Vector3& direction, float distance, float decay, float cosAngle, float cosFalloffStart) {
//    if (index < 0 || index >= kNumSpotLights) return;
//
//    lightData_.SpotLights[index].color = color;
//    lightData_.SpotLights[index].position = position;
//    lightData_.SpotLights[index].intensity = intensity;
//    lightData_.SpotLights[index].direction = direction;
//    lightData_.SpotLights[index].distance = distance;
//    lightData_.SpotLights[index].decay = decay;
//    lightData_.SpotLights[index].cosAngle = cosAngle;
//    lightData_.SpotLights[index].cosFalloffStart = cosFalloffStart;
//}
//
//void LightManager::CreateConstBufferResource() {
//    // データサイズ（256バイトアライメント調整）
//    size_t sizeInBytes = sizeof(LightGroupData);
//    sizeInBytes = (sizeInBytes + 0xff) & ~0xff;
//
//    // DXCommonを使ってリソース作成
//    constBufferResource_ = DXCommon::GetInstance()->CreateBufferResource(sizeInBytes);
//
//    // マッピング
//    constBufferResource_->Map(0, nullptr, reinterpret_cast<void**>(&constBufferData_));
//}

void LightManager::CreateStructuredBuffer(size_t sizeInBytes, Microsoft::WRL::ComPtr<ID3D12Resource>& resource) {
    // DXCommonのCreateBufferResourceはConstantBuffer用(256アライメント)になっている場合が多いですが、
    // StructuredBufferもUploadHeapで作るなら基本同じ関数でOKです。
    // ただしサイズは構造体サイズ * 個数でOK（256倍数である必要はないが、あっても問題ない）
    resource = DXCommon::GetInstance()->CreateBufferResource(sizeInBytes);
}

void LightManager::CreateConstBuffer(size_t sizeInBytes, Microsoft::WRL::ComPtr<ID3D12Resource>& resource)
{
     resource = DXCommon::GetInstance()->CreateBufferResource(sizeInBytes);
}
