#pragma once
#include <vector>
#include "Vector2.h"
#include "Vector4.h"
#include <wrl.h>
#include <d3d12.h>

class Model
{
public:
    struct VertexData {
        Vector4 position; // 4D position vector
        Vector2 texcoord; // 2D texture coordinate vector
        Vector3 normal;
    };
    struct Material {
        Vector4 color;
        int32_t enableLighting;
        int32_t diffuseType;  // 0:Lambert, 1:Half-Lambert
        int32_t specularType; // 0:None, 1:Phong, 2:BlinnPhong
        float padding[1];
        Matrix4x4 uvTransform; // UV変換行列
        float  shininess;
    };
    struct MaterialData {
        std::string textureFilePath;
        uint_fast16_t textureIndex = 0;
    };
    struct ModelData {
        std::vector<VertexData> vertices; // 頂点データの配列
        MaterialData material; // マテリアルデータ
    };
    enum  DiffuseType
    {
        Lambert,
        HarfLambert
    };
    enum  SpecularType {
        NONE,
        Phong,
        BlinnPhong,
    };

    void Initialize(const std::string& directryPath, const std::string& filename);

    void Draw();

    //マテリアルの読み込み
    static MaterialData LoadMaterialTemplateFile(const std::string& directryPath, const std::string& filename);
    //OBJファイルの読み込み
    static ModelData LoadObjFile(const std::string& directryPath, const std::string& filename);

    static Model* CreateSphere(uint32_t subdivision = 16);


private:

    ModelData modelData_;

    //頂点リソース
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourse_;
    VertexData* vertexData_ = nullptr;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
    void CreateVertexBuffer();
    //マテリアルリソース
    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
    Material* materialData_ = nullptr;
    void CreateMaterialResource();
};

