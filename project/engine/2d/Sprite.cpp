#include "Sprite.h"
#include "SpriteCommon.h"
#include "MassFunction.h"
void Sprite::Initialize(SpriteCommon* spriteCom) {

    spriteCom_ = spriteCom;

    vertexResourse_ =
        spriteCom_->GetDxCommon()->
        CreateBufferResource(sizeof(VertexData) * 4);
    indexResource_ =
        spriteCom_->GetDxCommon()->
        CreateBufferResource(sizeof(uint32_t) * 6);
    vertexBufferView_.BufferLocation =
        vertexResourse_.Get()->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
    vertexBufferView_.StrideInBytes = sizeof(VertexData);

    indexBufferView_.BufferLocation =
        indexResource_.Get()->GetGPUVirtualAddress();
    indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
    indexBufferView_.Format = DXGI_FORMAT_R32_UINT;//32ビット整数

    vertexResourse_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
    indexResource_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));

    //
    materialResource_ =
        spriteCom_->GetDxCommon()->
        CreateBufferResource(sizeof(Material));
    materialResource_->
        Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
    //データの設定
    materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    materialData_->enableLighting = false;
    materialData_->uvTransform = Makeidetity4x4();
    //座標変換
    transformationMatrixResourse_ =
        spriteCom_->GetDxCommon()->
        CreateBufferResource(sizeof(TransformationMatrix));
    transformationMatrixResourse_.Get()->
        Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
    transformationMatrixData_->WVP = Makeidetity4x4();
    transformationMatrixData_->World = Makeidetity4x4();
}
void Sprite::Update() {
    // 左下
    vertexData_[0].position = { 0.0f, 1.0f, 0.0f, 1.0f };
    vertexData_[0].texcoord = { 0.0f, 1.0f };
    vertexData_[0].normal = { 0.0f,0.0f, -1.0f };
    // 左上
    vertexData_[1].position = { 0.0f, 0.0f, 0.0f, 1.0f };
    vertexData_[1].texcoord = { 0.0f, 0.0f };
    vertexData_[1].normal = { 0.0f,0.0f, -1.0f };
    // 右下
    vertexData_[2].position = { 1.0f, 1.0f, 0.0f, 1.0f };
    vertexData_[2].texcoord = { 1.0f, 1.0f };
    vertexData_[2].normal = { 0.0f,0.0f, -1.0f };
    // 右上
    vertexData_[3].position = { 1.0f, 0.0f, 0.0f, 1.0f };
    vertexData_[3].texcoord = { 1.0f, 0.0f };
    vertexData_[3].normal = { 0.0f,0.0f, -1.0f };

    indexData_[0] = 0;
    indexData_[1] = 1;
    indexData_[2] = 2;
    indexData_[3] = 2;
    indexData_[4] = 1;
    indexData_[5] = 3;



    Transform transform{ {size_.x,size_.y,1.0f},{0.0f,0.0f,rotation_},{postion_.x,postion_.y,0.0f} };

     Matrix4x4 worldMatrix = MakeAfineMatrix(transform.scale, transform.rotate, transform.traslate);
    Matrix4x4 viewMatrix = Makeidetity4x4();
    Matrix4x4 projectionMatrix = MakeOrthographicMatrix(0.0f, 0.0f, static_cast<float>(WinApp::kClientWidth), static_cast<float>(WinApp::kClientHeight), 0.0f, 100.0f);
    //スプライトのワールド行列とビュー行列とプロジェクション行列を掛け算
    Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
    transformationMatrixData_->WVP = worldViewProjectionMatrix;
    transformationMatrixData_->World = worldMatrix;

}

void Sprite::Draw()
{
    spriteCom_->GetDxCommon()->
        GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);
    spriteCom_->GetDxCommon()->
        GetCommandList()->IASetIndexBuffer(&indexBufferView_);
   spriteCom_->GetDxCommon()->
        GetCommandList()->
        SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
    spriteCom_->GetDxCommon()->
        GetCommandList()->
        SetGraphicsRootConstantBufferView(1, transformationMatrixResourse_->GetGPUVirtualAddress());
    spriteCom_->GetDxCommon()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);
}
