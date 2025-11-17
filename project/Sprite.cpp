#include"Sprite.h"
#include"SpriteCommon.h"
#include "MassFunction.h"
#include "WinApp.h"


void Sprite::Initialize(SpriteCommon* spriteCommon) {
    spriteCommon_ = spriteCommon;

    vertexResourse_ = spriteCommon_->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * 4);

    indexResource_ = spriteCommon_->GetDxCommon()->CreateBufferResource(sizeof(uint32_t) * 6);

    //リソース先頭アドレス
    vertexBufferView_.BufferLocation = vertexResourse_.Get()->GetGPUVirtualAddress();
    //リソースのサイズ
    vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
    vertexBufferView_.StrideInBytes = sizeof(VertexData);

    //リソース先頭アドレス
    indexBufferView_.BufferLocation = indexResource_.Get()->GetGPUVirtualAddress();
    //リソースのサイズ
    indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
    indexBufferView_.Format = DXGI_FORMAT_R32_UINT;//32ビット整数

    vertexResourse_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

    indexResource_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));
    //
    materialResource_ = spriteCommon_->GetDxCommon()->CreateBufferResource(sizeof(Material));
    materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

    materialData_->color = Vector4{ 1.0f,1.0f,1.0f,1.0f };
    materialData_->enableLighting = false;
    materialData_->uvTransform = Makeidetity4x4();
    //
    transformationMatrixResourse_ = spriteCommon_->GetDxCommon()->CreateBufferResource(sizeof(TransformationMatrix));
    transformationMatrixResourse_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
    //行列の初期化
    transformationMatrixData_->WVP = Makeidetity4x4();
    transformationMatrixData_->World = Makeidetity4x4();
}

void Sprite::Update(){

     Transform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
     Matrix4x4 worldMatrix = MakeAfineMatrix(transform.scale, transform.rotate, transform.traslate);
        Matrix4x4 viewMatrix = Makeidetity4x4();
        Matrix4x4 projectionMatrix = MakeOrthographicMatrix(0.0f,0.0f,static_cast<float>(WinApp::kClientWidth),static_cast<float>(WinApp::kClientHeight),0.0f,100.0f);
        transformationMatrixData_->WVP =  Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));;
        transformationMatrixData_->World=worldMatrix;
}