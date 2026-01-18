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
    CreateConstBufferResource();

    // 初期値の設定（全て無効化状態で初期化）
    for (int i = 0; i < kNumDirectionalLights; ++i) {
        lightData_.directionalLights[i].color = { 1.0f, 1.0f, 1.0f, 1.0f };
        lightData_.directionalLights[i].direction = { 0.0f, -1.0f, 0.0f };
        lightData_.directionalLights[i].intensity = 0.0f;
    }
    for (int i = 0; i < kNumPointLights; ++i) {
        lightData_.PointLightLights[i].color = { 1.0f, 1.0f, 1.0f, 1.0f };
        lightData_.PointLightLights[i].radius = 0.0f;
        lightData_.PointLightLights[i].intensity = 0.0f;
        lightData_.PointLightLights[i].position = { 0.0f, 0.0f, 0.0f };
        lightData_.PointLightLights[i].decay = 0.1f;
    }
    for (int i = 0; i < kNumSpotLights; ++i) {
        lightData_.SpotLights[i].color = { 1.0f, 1.0f, 1.0f, 1.0f }; // 白
        lightData_.SpotLights[i].position = { 2.0f, 1.25f, 0.0f };    // 座標 (Y=2.0)
        lightData_.SpotLights[i].distance = 7.0f;
        lightData_.SpotLights[i].direction = Normalize({ -1.0f,-1.0f,0.0f });
        lightData_.SpotLights[i].intensity = 0.0f;  // 強度
        lightData_.SpotLights[i].decay = 2.0f;
        lightData_.SpotLights[i].cosAngle = 0.0f;
        lightData_.SpotLights[i].cosFalloffStart = 0.0f;
    }
}

void LightManager::Update() {


    // 定数バッファへコピー
    if (constBufferData_) {
        for (int i = 0; i < kNumDirectionalLights; ++i) {
            constBufferData_->directionalLights[i] = lightData_.directionalLights[i];
        }
        for (int i = 0; i < kNumPointLights; ++i) {
            constBufferData_->PointLightLights[i] = lightData_.PointLightLights[i];
        }
        for (int i = 0; i < kNumDirectionalLights; ++i) {
            constBufferData_->directionalLights[i] = lightData_.directionalLights[i];
        }
    }
}

void LightManager::Draw(UINT rootParameterIndex) {
    if (!constBufferResource_) return;

    // コマンドリストに定数バッファをセット
    DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(
        rootParameterIndex,
        constBufferResource_->GetGPUVirtualAddress()
    );
}

void LightManager::SetDirectionalLight(int index, const Vector4& color, const Vector3& direction, float intensity) {
    if (index < 0 || index >= kNumDirectionalLights) return;

    lightData_.directionalLights[index].color = color;
    lightData_.directionalLights[index].direction = direction;
    lightData_.directionalLights[index].intensity = intensity;
}

void LightManager::CreateConstBufferResource() {
    // データサイズ（256バイトアライメント調整）
    size_t sizeInBytes = sizeof(LightGroupData);
    sizeInBytes = (sizeInBytes + 0xff) & ~0xff;

    // DXCommonを使ってリソース作成
    constBufferResource_ = DXCommon::GetInstance()->CreateBufferResource(sizeInBytes);

    // マッピング
    constBufferResource_->Map(0, nullptr, reinterpret_cast<void**>(&constBufferData_));
}