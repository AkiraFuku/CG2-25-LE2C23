#pragma once
#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <memory>
#include "Vector4.h"
#include "Vector3.h"

// 定数：扱える最大ライト数
const int kNumDirectionalLights = 3;
const int kNumPointLights = 3;
const int kNumSpotLights = 3;

class LightManager
{
public: // メンバ関数
    // シングルトンインスタンスの取得
    static LightManager* GetInstance();

    // 初期化
    void Initialize();
    // 更新
    void Update();
    // 描画設定（コマンドリストへのバインド）
    void Draw(UINT rootParameterIndex);

    // 終了処理（明示的に解放したい場合）
    void Finalize();

    // ライトの設定
    void SetDirectionalLight(int index, const Vector4& color, const Vector3& direction, float intensity);

private: // メンバ変数・内部定義
    // シングルトンパターンのためコンストラクタを隠蔽
    LightManager() = default;
    ~LightManager() = default;
    LightManager(const LightManager&) = delete;
    LightManager& operator=(const LightManager&) = delete;

    // 定数バッファ作成関数
    void CreateConstBufferResource();

    // 定数バッファ用構造体 (HLSL側と合わせる)
    struct DirectionalLightData {
        Vector4 color;      // color
        Vector3 direction;  // direction
        float intensity;    // intensity
    };
    struct PointLight {
        Vector4 color;//ライトの色
        Vector3 position;//ライトの向き
        float intensity;// 明るさ
        float radius;
        float decay;
        float padding[2];
    };
    struct SpotLight {
        Vector4 color;//ライトの色
        Vector3 position;//ライトの向き
        float intensity;// 明るさ
        Vector3 direction;
        float distance;
        float decay;
        float cosAngle;
        float cosFalloffStart;
        float padding;
    };
    struct LightGroupData {
        DirectionalLightData directionalLights[kNumDirectionalLights];
        SpotLight SpotLights[kNumSpotLights];
        PointLight PointLightLights[kNumPointLights];
    };

    // スマートポインタでリソース管理
    Microsoft::WRL::ComPtr<ID3D12Resource> constBufferResource_;

    // マップ先のポインタ（リソース解放時に自動的に無効になるため生ポインタでOKだが、管理には注意）
    LightGroupData* constBufferData_ = nullptr;

    // CPU側のライトデータ保持用
    LightGroupData lightData_;

    // シングルトンインスタンス
    static std::unique_ptr<LightManager> instance;
    friend struct std::default_delete<LightManager>;
};
