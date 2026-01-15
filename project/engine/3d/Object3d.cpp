#include "Object3d.h"
#include "Object3dCommon.h"
#include <cassert>
#include <fstream> // 追加: ifstreamの完全な型を利用するため
#include <sstream> // 追加: istringstreamのため
#include "MathFunction.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include <imgui.h>


void Object3d::Initialize()
{

    //WVP行列リソースの作成
    CreateWVPResource();
    //平行光源リソースの作成
    CreateDirectionalLightResource();
    transform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
    camera_ = Object3dCommon::GetInstance()->GetDefaultCamera();
    CreateCameraResource();
}
void Object3d::Update()
{
    model_->Update();
    //  WVP行列の作成
    Matrix4x4 worldMatrix = MakeAfineMatrix(transform_.scale, transform_.rotate, transform_.translate);
    Matrix4x4 worldViewProjectionMatrix = {};
    //ワールド行列とビュー行列とプロジェクション行列を掛け算
    if (camera_)
    {
        cameraData_->worldPosition = camera_->GetTranslate();
        worldViewProjectionMatrix = Multiply(worldMatrix, camera_->GetViewProtectionMatrix());
    } else {
        worldViewProjectionMatrix = worldMatrix;
    }
    //行列をGPUに転送
    wvpResource_->WVP = worldViewProjectionMatrix;
    wvpResource_->World = worldMatrix;
    directionalLightData_->direction = Normalize(directionalLightData_->direction);
}

void Object3d::Draw()
{
    Object3dCommon::GetInstance()->Object3dCommonDraw();
    PsoProperty psoProp = { PipelineType::Object3d, blendMode_ ,fillMode_ };
    PsoSet psoSet = PSOMnager::GetInstance()->GetPsoSet(psoProp);

    // PSOをセット
    DXCommon::GetInstance()->GetCommandList()->SetPipelineState(psoSet.pipelineState.Get());
    //WVP行列リソースの設定
    DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_.Get()->GetGPUVirtualAddress());
    //light
    DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource_.Get()->GetGPUVirtualAddress());
    DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(4, cameraResource_->GetGPUVirtualAddress());
    if (model_) {
        model_->Draw();
    }
}

void Object3d::SetModel(const std::string& filePath)
{
    model_ = ModelManager::GetInstance()->findModel(filePath);
}


void Object3d::CreateWVPResource()
{
    //座標変換
    transformationMatrixResource_ =
        DXCommon::GetInstance()->
        CreateBufferResource(sizeof(TransformationMatrix));
    transformationMatrixResource_.Get()->
        Map(0, nullptr, reinterpret_cast<void**>(&wvpResource_));
    wvpResource_->WVP = Makeidetity4x4();
    wvpResource_->World = Makeidetity4x4();
    wvpResource_->WorldInverseTranspose=Inverse( wvpResource_->World );
}

void Object3d::CreateDirectionalLightResource()
{
    directionalLightResource_ =
        DXCommon::GetInstance()->
        CreateBufferResource(sizeof(DirectionalLight));
    directionalLightResource_.Get()->
        Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
    directionalLightData_->color = { 1.0f,1.0f,1.0f,1.0f };
    directionalLightData_->direction = { 0.0f,-1.0f,0.0f };
    directionalLightData_->intensity = 1.0f;

}

void Object3d::CreateCameraResource()
{
    cameraResource_ =
        DXCommon::GetInstance()->
        CreateBufferResource( (sizeof(CameraForGPU) + 0xff) & ~0xff);
    cameraResource_.Get()->
        Map(0, nullptr, reinterpret_cast<void**>(&cameraData_));
    if (camera_!=nullptr)
    {
        cameraData_->worldPosition=camera_->GetTranslate();
    } else
    {
        cameraData_->worldPosition=Vector3{1.0f,1.0f,1.0f};
    }

}
