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
