#pragma once
#include "Vector4.h"
#include "Vector2.h"
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
    struct Material
    {
        Vector4 color;
        int32_t enableLighting;
        float padding[3]; // パディングを追加してサイズを揃える
        Matrix4x4 uvTransform; // UV変換行列

    };
    struct TransformationMatrix
    {
        Matrix4x4 WVP;
        Matrix4x4 World;

    };

    void Initialize(SpriteCommon* spriteCom);
    void Update();
    void Draw();

    const Vector2& GetPosition() const{return postion_;} 
    void SetPosition(const Vector2& position){postion_=position;}

    float GetRotation() const{return rotation_;}
    void SetRotation(const float rotation){rotation_=rotation;}
    
private:
    SpriteCommon* spriteCom_ = nullptr;

    Vector2 postion_={0.0f,0.0f};
    float rotation_=0.0f;

    //buffer
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourse_;
    Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
    VertexData* vertexData_ = nullptr;
    uint32_t* indexData_ = nullptr;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
    D3D12_INDEX_BUFFER_VIEW indexBufferView_;
    //マテリアル
    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
    Material* materialData_ = nullptr;
    //座標変換
    Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourse_;
    TransformationMatrix* transformationMatrixData_ = nullptr;
};

