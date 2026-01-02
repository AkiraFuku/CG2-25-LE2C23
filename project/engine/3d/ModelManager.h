#pragma once
#include <map>
#include <Model.h>
#include <memory>
#include "DXCommon.h"
class ModelCommon;
class ModelManager
{
public:
    void Initialize(DXCommon* dxCommon);
    static ModelManager* GetInstance();
    void Finalize();
    //Modelロード
    void LoadModel(const std::string& filePath);
    //Model検索
    std::shared_ptr<Model> findModel(const std::string& filePath);
private:
    static std::unique_ptr<ModelManager> instance;
    std::unique_ptr<ModelCommon> modelCommon_;

    ModelManager() = default;
    ~ModelManager() = default;
    ModelManager(ModelManager&) = delete;
    ModelManager& operator=(ModelManager&) = delete;

    std::map<std::string, std::shared_ptr<Model>> models;
};

