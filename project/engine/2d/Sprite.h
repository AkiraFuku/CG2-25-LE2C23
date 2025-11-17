#pragma once
#include <wrl.h>
#include "Vector4.h"
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



    void Initialize(SpriteCommon* spriteCommon);

    void Update();

    void Draw();
private:
    SpriteCommon* spriteCommon_ = nullptr;

    //バッファリソース
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourse_;
    Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
    //　バッファリソースのデータのポインター
    VertexData* vertexData_ = nullptr;
    uint32_t* indexData_ = nullptr;
    // バッファビュー
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
    D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

    //マテリアル
    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
    Material* materialData_ = nullptr;
    //座標変換
     Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourse_;
      //スプライトの行列データの設定
    TransformationMatrix* transformationMatrixData_;

};

