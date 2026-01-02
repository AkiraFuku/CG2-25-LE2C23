#pragma once
#include "d3d12.h"
#include <wrl/client.h>
#include <cstdint>
#include "DXCommon.h"
#include "Camera.h"
class Object3dCommon
{
public:

    // シングルトン化
    static Object3dCommon* GetInstance();
    void Finalize();

    void Initialize(DXCommon* dxCommon);
    DXCommon* GetDxCommon()const {return dxCommon_;}
    void Object3dCommonDraw();
    void SetDefaultCamera(Camera* camera) {
        defaultCamera_ = camera;
    }
    Camera* GetDefaultCamera()const {
        return defaultCamera_;
    }

private:

    // シングルトンパターン
    static std::unique_ptr<Object3dCommon> instance;
    Object3dCommon() = default;
    ~Object3dCommon() = default;
    Object3dCommon(const Object3dCommon&) = delete;
    Object3dCommon& operator=(const Object3dCommon&) = delete;

    HRESULT hr_;

    DXCommon* dxCommon_=nullptr;

    //ルートシグネチャ
    Microsoft::WRL::ComPtr<ID3D12RootSignature>rootSignature_;
    void CreateRootSignature();
    //グラフィックパイプラインステート
    Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;
    void CreatePSO();
    Camera* defaultCamera_ = nullptr;
};

