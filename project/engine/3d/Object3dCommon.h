#pragma once
#include "d3d12.h"
#include <wrl/client.h>
#include <cstdint>
#include "DXCommon.h"
#include "Camera.h"
class Object3dCommon
{
public:
    void Initialize(DXCommon* dxCommon);
    DXCommon* GetDxCommon()const {
        return dxCommon_;
    }

    void Object3dCommonDraw();
    void SetDefaultCamera(Camera* camera) {
        defaultCamera_ = camera;
    }
    Camera* GetDefaultCamera()const {
        return defaultCamera_;
    }

private:
    HRESULT hr_;

    DXCommon* dxCommon_;

    //ルートシグネチャ
    Microsoft::WRL::ComPtr<ID3D12RootSignature>rootSignature_;
    void CreateRootSignature();
    //グラフィックパイプラインステート
    Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;
    void CreatePSO();
    Camera* defaultCamera_ = nullptr;
};

