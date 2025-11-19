#pragma once
#include "Vector4.h"
#include <wrl.h>
#include <d3d12.h>

class SpriteCommon;
class Sprite
{
public:
    struct VertexData {
        Vector4 position; // 4D position vector
        Vector2 texcoord; // 2D texture coordinate vector
        Vector3 normal;
    };

    void Initialize(SpriteCommon* spriteCom);
private:
    SpriteCommon* spriteCom_ = nullptr;
    //buffer
     Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourse_;
    Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
    VertexData* vertexData_=nullptr;
    uint32_t* indexData_=nullptr;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
    D3D12_INDEX_BUFFER_VIEW indexBufferView_;
};

