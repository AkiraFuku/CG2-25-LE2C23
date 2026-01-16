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
        lightData_.lights[i].color = { 1.0f, 1.0f, 1.0f, 1.0f };
        lightData_.lights[i].direction = { 0.0f, -1.0f, 0.0f };
        lightData_.lights[i].intensity = 0.0f;
    }
}

void LightManager::Update() {
    // データの転送
    for (int i = 0; i < kNumDirectionalLights; ++i) {
        // 必要なら方向の正規化などを行う
        // lightData_.lights[i].direction = Normalize(lightData_.lights[i].direction);
    }

    // 定数バッファへコピー
    if (constBufferData_) {
        for (int i = 0; i < kNumDirectionalLights; ++i) {
            constBufferData_->lights[i] = lightData_.lights[i];
        }
    }
}

void LightManager::Draw( UINT rootParameterIndex) {
    if ( !constBufferResource_) return;

    // コマンドリストに定数バッファをセット
   DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(
        rootParameterIndex, 
        constBufferResource_->GetGPUVirtualAddress()
    );
}

void LightManager::SetDirectionalLight(int index, const Vector4& color, const Vector3& direction, float intensity) {
    if (index < 0 || index >= kNumDirectionalLights) return;
    
    lightData_.lights[index].color = color;
    lightData_.lights[index].direction = direction;
    lightData_.lights[index].intensity = intensity;
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