#pragma once
#include <Vector4.h>
#include <Vector2.h>
#include <wrl.h>
#include <d3d12.h>
#include <string>
#include <vector>
#include "Model.h"
#include "Camera.h"
#include "Object3dCommon.h"
#include "PSOMnager.h"
class Object3d
{

public:

    struct TransformationMatrix {
        Matrix4x4 WVP;
        Matrix4x4 World;

    };
    struct DirectionalLight {
        Vector4 color;//ライトの色
        Vector3 direction;//ライトの向き
        float intensity;// 明るさ


    };
    struct CameraForGPU
    {
        Vector3 worldPosition;
    };
    void Initialize();
    void Update();
    void Draw();
    void SetModel(const std::string& filePath);

    //トランスフォームセッター
    void SetScale(const Vector3& scale) {
        transform_.scale = scale;
    }
    void SetRotate(const Vector3& rotate) {
        transform_.rotate = rotate;
    }
    void SetTranslate(const Vector3& translate) {
        transform_.translate = translate;
    }

    void SetDirectionalLight(const DirectionalLight& light) {
        if (directionalLightData_) {
            // メモリコピーで値を一括更新
            *directionalLightData_ = light;
        }
    }
    // ライトのセッター
    void SetDirectionalLightColor(const Vector4& color) {
        if (directionalLightData_) {
            directionalLightData_->color = color;
        }
    }
    void SetDirectionalLightDirection(const Vector3& direction) {
        if (directionalLightData_) {
            directionalLightData_->direction = direction;
        }
    }
    void SetDirectionalLightIntensity(float intensity) {
        if (directionalLightData_) {
            directionalLightData_->intensity = intensity;
        }
    }

    void SetCamera(Camera* camera) {
        camera_ = camera;
    }


    //トランスフォームゲッター
    const Vector3& GetScale()const {
        return transform_.scale;
    }
    const Vector3& GetRotate()const {
        return transform_.rotate;
    }
    const Vector3& GetTranslate()const {
        return transform_.translate;
    }
    // ライトのゲッター
    const Vector4& GetDirectionalLightColor() const {
        return directionalLightData_->color;
    }
    const Vector3& GetDirectionalLightDirection() const {
        return directionalLightData_->direction;
    }
    float GetDirectionalLightIntensity() const {
        return directionalLightData_->intensity;
    }
    void SetBlendMode(BlendMode blendMode) {
        blendMode_ = blendMode;
    }
    void SetFillMode(FillMode fillMode) {
        fillMode_ = fillMode;
    }
private:


    std::shared_ptr<Model> model_ = nullptr;
    //WVP行列リソース
    Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;
    TransformationMatrix* wvpResource_ = nullptr;
    void CreateWVPResource();
    //平行光源
    Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_;
    DirectionalLight* directionalLightData_ = nullptr;
    void CreateDirectionalLightResource();

    Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource_;
     CameraForGPU* cameraData_ = nullptr;
     void  CreateCameraResource();
    //トランスフォーム
    Transform transform_ = {};
    //カメラ　
    Camera* camera_ = nullptr;

    FillMode fillMode_ = FillMode::kSolid;
    BlendMode blendMode_ = BlendMode::None;
};

