#pragma once
#include"Data.h"
#include <string>
#include <vector>
class Object3dCommon;
class Object3d
{

public:
    struct MaterialData
    {
        std::string textureFilePath;
    };
    struct ModelData {
        std::vector<VertexData> vertices; // 頂点データの配列
        MaterialData material; // マテリアルデータ
    };

    void Initialize(Object3dCommon* object3dCommon);
    //マテリアルの読み込み
    static MaterialData LoadMaterialTemplateFile(const std::string& directryPath, const std::string& filename);
    //OBJファイルの読み込み
    static ModelData LoadObjFile(const std::string& directryPath, const std::string& filename) ;
    

private:
    Object3dCommon* object3dCommon_ = nullptr;

    ModelData modelData_;

};

