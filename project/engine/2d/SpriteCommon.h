#pragma once
#include "d3d12.h"
#include <wrl.h>
#include <cstdint>
#include "DXCommon.h"

class SpriteCommon
{
public:
    void Initialize(DXCommon* dxCommon);

    DXCommon* GetDxCommon()const {
        return dxCommon_;
    }
    void SpriteCommonDraw();


private:
    HRESULT hr_;

    DXCommon* dxCommon_;

    //ルートシグネチャ
    Microsoft::WRL::ComPtr<ID3D12RootSignature>rootSignature_;
    void CreateRootSignature();
    //グラフィックパイプラインステート
    Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;
    void CreatePSO();



};

