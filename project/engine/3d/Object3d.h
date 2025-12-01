#pragma once
#include <Vector4.h>
#include <Vector2.h>
#include <wrl.h>
#include <d3d12.h>

#include <string>
#include <vector>
class Object3dCommon;
class Object3d
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
    struct MaterialData
    {
        std::string textureFilePath;
    };
    struct ModelData {
        std::vector<VertexData> vertices; // 頂点データの配列
        MaterialData material; // マテリアルデータ
    };
    struct TransformationMatrix
    {
        Matrix4x4 WVP;
        Matrix4x4 World;

    };

    void Initialize(Object3dCommon* object3dCommon);
    //マテリアルの読み込み
    static MaterialData LoadMaterialTemplateFile(const std::string& directryPath, const std::string& filename);
    //OBJファイルの読み込み
    static ModelData LoadObjFile(const std::string& directryPath, const std::string& filename);


private:
    Object3dCommon* object3dCom_ = nullptr;

    ModelData modelData_;

    //頂点リソース
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourse_;
    VertexData* vertexData_ = nullptr;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
    //マテリアルリソース
    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
    Material* materialData_ = nullptr;
};

