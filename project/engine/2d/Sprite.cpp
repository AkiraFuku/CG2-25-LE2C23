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