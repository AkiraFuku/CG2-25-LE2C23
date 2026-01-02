#include "ModelManager.h"
#include "ModelCommon.h"
std::unique_ptr<ModelManager> ModelManager::instance = nullptr;
void ModelManager::Initialize(DXCommon* dxCommon) {
    modelCommon_ = std::make_unique<ModelCommon>();
    modelCommon_->Initialize(dxCommon);

}
ModelManager* ModelManager::GetInstance() {
    if (instance == nullptr) {
        instance.reset(new ModelManager());
    }
    return instance.get();

};
void ModelManager::Finalize() {

    models.clear(); 
    modelCommon_.reset(); // 明示的に解放する場合
    instance.reset();
}

void ModelManager::LoadModel(const std::string& filePath)
{
    //読み込み済か確認
    if (models.contains(filePath))return;
    //読み込み.初期化
   std::shared_ptr<Model> model = std::make_shared<Model>();
    model->Initialize(modelCommon_.get(), "resources", filePath);
    //格納
    models.insert(std::make_pair(filePath, std::move(model)));


}

std::shared_ptr<Model> ModelManager::findModel(const std::string& filePath)
{
    if (models.contains(filePath)) {
        return models.at(filePath);
    }


    LoadModel(filePath);
    if (models.contains(filePath)) {
        return models.at(filePath);
    }
    return nullptr;
}
