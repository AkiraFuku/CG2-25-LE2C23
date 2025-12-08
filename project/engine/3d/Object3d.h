#pragma once
#include <Vector4.h>
#include <Vector2.h>
#include <wrl.h>
#include <d3d12.h>
#include <string>
#include <vector>
#include "Model.h"
class Object3dCommon;
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

    void Initialize(Object3dCommon* object3dCommon);
    void Update();
    void Draw();
    void SetModel(Model* model) {
        model_ = model;
    }

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

private:
    Object3dCommon* object3dCom_ = nullptr;

    Model* model_ = nullptr;
    //WVP行列リソース
    Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourse_;
    TransformationMatrix* wvpResource_ = nullptr;
    void CreateWVPResource();
    //平行光源
    Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResourse_;
    DirectionalLight* directionalLightData_ = nullptr;
    void CreateDirectionalLightResource();

    //トランスフォーム
    Transform transform_;
    Transform cameraTransform_;

};

