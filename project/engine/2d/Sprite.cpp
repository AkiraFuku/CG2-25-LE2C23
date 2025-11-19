#include "Sprite.h"
#include "SpriteCommon.h"

void Sprite::Initialize(SpriteCommon* spriteCom){

    spriteCom_=spriteCom;

    vertexResourse_=
        spriteCom_->GetDxCommon()->
        CreateBufferResource(sizeof(VertexData) * 4);
    indexResource_=
        spriteCom_->GetDxCommon()->
        CreateBufferResource(sizeof(uint32_t) * 6);
    vertexBufferView_.BufferLocation=
        vertexResourse_.Get()->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes= sizeof(VertexData) * 4;
    vertexBufferView_.StrideInBytes=sizeof(VertexData);

    indexBufferView_.BufferLocation=
        indexResource_.Get()->GetGPUVirtualAddress();
    indexBufferView_.SizeInBytes=sizeof(uint32_t) * 6;
    indexBufferView_.Format= DXGI_FORMAT_R32_UINT;//32ビット整数

    vertexResourse_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
    indexResource_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));

}