#pragma once
#include "d3d12.h"
#include <wrl/client.h>
#include <cstdint>
#include "DXCommon.h"
#include <memory>
class SpriteCommon
{
public:
  // シングルトン化
    static SpriteCommon* GetInstance();
    void Finalize();

    void Initialize(DXCommon* dxCommon);

    DXCommon* GetDxCommon()const {
        return dxCommon_;
    }
    void SpriteCommonDraw();


private:
    // シングルトンパターン
    static std::unique_ptr<SpriteCommon> instance;
    SpriteCommon() = default;
    ~SpriteCommon() = default;
    SpriteCommon(const SpriteCommon&) = delete;
    SpriteCommon& operator=(const SpriteCommon&) = delete;
    HRESULT hr_;

    DXCommon* dxCommon_=nullptr;

    //ルートシグネチャ
    Microsoft::WRL::ComPtr<ID3D12RootSignature>rootSignature_;
    void CreateRootSignature();
    //グラフィックパイプラインステート
    Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;
    void CreatePSO();



};

