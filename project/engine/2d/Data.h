#pragma once
#include <Vector4.h>
struct Transform{
    Vector3 scale;
    Vector3 rotate;
    Vector3 traslate;
};
struct VertexData{
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

struct DirectionalLight{
    Vector4 color;//ライトの色
    Vector3 direction;//ライトの向き
    float intensity;// 明るさ


};
struct MaterialData
{
    std::string textureFilePath;
};
struct ModelData{
    std::vector<VertexData> vertices; // 頂点データの配列
    MaterialData material; // マテリアルデータ
};
