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
private:
    static ModelManager* instance;
    ModelCommon* modelCommon_=nullptr;

    ModelManager() = default;
    ~ModelManager() = default;
    ModelManager(ModelManager&) = delete;
    ModelManager& operator=(ModelManager&) = delete;

    std::map<std::string, std::unique_ptr<Model>> models;
};

