#include "Object3d.h"
#include "Object3dCommon.h"
#include <cassert>
#include <fstream> // 追加: ifstreamの完全な型を利用するため
#include <sstream> // 追加: istringstreamのため
#include "MassFunction.h"
#include "TextureManager.h"


void Object3d::Initialize(Object3dCommon* object3dCommon)
{
    object3dCom_ = object3dCommon;


    //WVP行列リソースの作成
    CreateWVPResource();
    //平行光源リソースの作成
    CreateDirectionalLightResource();


    transform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
    cameraTransform_ = { {1.0f,1.0f,1.0f},{0.3f,0.0f,0.0f},{0.0f,4.0f,-10.0f} };
}
void Object3d::Update()
{
    //  WVP行列の作成
    Matrix4x4 worldMatrix = MakeAfineMatrix(transform_.scale, transform_.rotate, transform_.translate);
    Matrix4x4 cameraMatrix = MakeAfineMatrix(cameraTransform_.scale, cameraTransform_.rotate, cameraTransform_.translate);
    Matrix4x4 viewMatrix = Inverse(cameraMatrix);
    //透視投影行列の作成
    Matrix4x4 projectionMatirx = MakePerspectiveFovMatrix(
        0.45f, static_cast<float>(WinApp::kClientWidth) / static_cast<float>(WinApp::kClientHeight), 0.1f, 100.0f
    );
    //ワールド行列とビュー行列とプロジェクション行列を掛け算
    Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatirx));
    //行列をGPUに転送
    wvpResource_->WVP = worldViewProjectionMatrix;
    wvpResource_->World = worldMatrix;
}

void Object3d::Draw()
{
    //WVP行列リソースの設定
    object3dCom_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResourse_.Get()->GetGPUVirtualAddress());
    //light
    object3dCom_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResourse_.Get()->GetGPUVirtualAddress());

    if (model_){
        model_->Draw();
    }
}

void Object3d::CreateWVPResource()
{
    //座標変換
    transformationMatrixResourse_ =
        object3dCom_->GetDxCommon()->
        CreateBufferResource(sizeof(TransformationMatrix));
    transformationMatrixResourse_.Get()->
        Map(0, nullptr, reinterpret_cast<void**>(&wvpResource_));
    wvpResource_->WVP = Makeidetity4x4();
    wvpResource_->World = Makeidetity4x4();

}

void Object3d::CreateDirectionalLightResource()
{
    directionalLightResourse_ =
        object3dCom_->GetDxCommon()->
        CreateBufferResource(sizeof(DirectionalLight));
    directionalLightResourse_.Get()->
        Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
    directionalLightData_->color = { 1.0f,1.0f,1.0f,1.0f };
    directionalLightData_->direction = { 0.0f,-1.0f,0.0f };
    directionalLightData_->intensity = 1.0f;

}
