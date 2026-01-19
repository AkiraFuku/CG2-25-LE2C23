#pragma once
#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <memory>
#include "Vector4.h"
#include "Vector3.h"
#include <vector>
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


    // ライト追加関数
    void AddDirectionalLight(const Vector4& color, const Vector3& direction, float intensity);
    void AddPointLight(const Vector4& color, const Vector3& position, float intensity, float radius, float decay);
    void AddSpotLight(const Vector4& color, const Vector3& position, float intensity, const Vector3& direction, float distance, float decay, float cosAngle, float cosFalloffStart);
    
    void ClearLights(); // 毎フレームリセット用

    //// ライトの設定
    //void SetDirectionalLight(int index, const Vector4& color, const Vector3& direction, float intensity);

    //void SetPointLight(int index, const Vector4& color, const Vector3& position, float intensity, float radius, float decay);
    //void SetSpotLight(int index, const Vector4& color, const Vector3& position, float intensity, const Vector3& direction, float distance, float decay, float cosAngle, float cosFalloffStart);

    //Vector3 GetPointPos(int index){return lightData_.PointLightLights[index].position;}
    //void SetPointPos(int index,Vector3 pos){lightData_.PointLightLights[index].position=pos;}

private: // メンバ変数・内部定義
    // シングルトンパターンのためコンストラクタを隠蔽
    LightManager() = default;
    ~LightManager() = default;
    LightManager(const LightManager&) = delete;
    LightManager& operator=(const LightManager&) = delete;

    //// 定数バッファ作成関数
    //void CreateConstBufferResource();

    // 定数バッファ用構造体 (HLSL側と合わせる)
    struct DirectionalLightData {
        Vector4 color;      // color
        Vector3 direction;  // direction
        float intensity;    // intensity
    };
    struct PointLightData {
        Vector4 color;//ライトの色
        Vector3 position;//ライトの向き
        float intensity;// 明るさ
        float radius;
        float decay;
        float padding[2];
    };
    struct SpotLightData {
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
  /*  struct LightGroupData {
        DirectionalLightData directionalLights[kNumDirectionalLights];
        SpotLight SpotLights[kNumSpotLights];
        PointLight PointLightLights[kNumPointLights];
    };*/
    struct LightCounts {
        int numDirectional;
        int numPoint;
        int numSpot;
        int padding;
    };

    // リソース作成ヘルパー
    void CreateStructuredBuffer(size_t sizeInBytes, Microsoft::WRL::ComPtr<ID3D12Resource>& resource);
    void CreateConstBuffer(size_t sizeInBytes, Microsoft::WRL::ComPtr<ID3D12Resource>& resource);
    //// スマートポインタでリソース管理
    //Microsoft::WRL::ComPtr<ID3D12Resource> constBufferResource_;

    //// マップ先のポインタ（リソース解放時に自動的に無効になるため生ポインタでOKだが、管理には注意）
    //LightGroupData* constBufferData_ = nullptr;

    //// CPU側のライトデータ保持用
    //LightGroupData lightData_;


    // データ保持用
    std::vector<DirectionalLightData> directionalLights_;
    std::vector<PointLightData> pointLights_;
    std::vector<SpotLightData> spotLights_;

    // GPUリソース
    Microsoft::WRL::ComPtr<ID3D12Resource> dirLightBuff_;
    Microsoft::WRL::ComPtr<ID3D12Resource> pointLightBuff_;
    Microsoft::WRL::ComPtr<ID3D12Resource> spotLightBuff_;
    Microsoft::WRL::ComPtr<ID3D12Resource> countBuff_;

    // カウントバッファマップ用
    LightCounts* countData_ = nullptr;
    // シングルトンインスタンス
    static std::unique_ptr<LightManager> instance;
    friend struct std::default_delete<LightManager>;
};
