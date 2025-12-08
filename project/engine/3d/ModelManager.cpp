#include "ModelManager.h"
#include "ModelCommon.h"
ModelManager* ModelManager::instance = nullptr;
void ModelManager::Initialize(DXCommon* dxCommon){
    modelCommon_=new ModelCommon;
    modelCommon_->Initialize(dxCommon);

}
ModelManager* ModelManager::GetInstance() {
    if (instance == nullptr)
    {
        instance = new ModelManager;
    }
    return instance;

};
void ModelManager::Finalize(){

    delete instance;
    instance=nullptr;
}

void ModelManager::LoadModel(const std::string& filePath)
{
    //読み込み済か確認
    if (models.contains(filePath))return;
    //読み込み.初期化
    std::unique_ptr<Model>model=std::make_unique<Model>();
    model->Initialize(modelCommon_,"resources",filePath);
    //格納
    models.insert(std::make_pair(filePath,std::move(model)));

    
}
