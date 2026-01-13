#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "Vector4.h" // 定義した構造体をインクルード
class LightManager
{
public:
    // 初期化
    void Initialize(ID3D12Device* device);
    
    // 描画ごとの更新（GPUへ転送）
    void TransferBuffer();

    // コマンドリストへの設定（描画時）
    // rootParameterIndex はルートシグネチャで b1 を設定しているインデックス
    void SetGraphicsRootConstantBufferView(ID3D12GraphicsCommandList* commandList, UINT rootParameterIndex);

    // ライトの登録・設定関数
    void SetDirectionalLight(int index, const DirectionalLight& light);
    void SetPointLight(int index, const PointLight& light);
    void SetSpotLight(int index, const SpotLight& light);

    // アクティブなライトの数を設定
    void SetNumDirectionalLights(int num) { data_.numDirLights = num; }
    void SetNumPointLights(int num) { data_.numPointLights = num; }
    void SetNumSpotLights(int num) { data_.numSpotLights = num; }

private:
    // 定数バッファ生成ヘルパー
    void CreateConstantBuffer(ID3D12Device* device);

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> constantBuffer_;
    LightGroupData* mappedData_ = nullptr;
    LightGroupData data_; // CPU側のデータ保持用
};

