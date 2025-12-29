#include "SceneManager.h"

SceneManager::~SceneManager()
{
    if (scene_) {
        scene_->Finalize();
        delete scene_;
        scene_ = nullptr;
    }
}

void SceneManager::Update() {
    // シーン切り替え処理
    if (nextScene_){

        if (scene_)
        {
            scene_->Finalize();
            delete scene_;
            scene_ = nullptr;

        }
        scene_ = nextScene_;
        nextScene_ = nullptr;

        scene_->SetSceneManager(this);
        scene_->Initialize();

    }
   
    if (scene_ != nullptr) {
        scene_->Update();
    }
}
void SceneManager::Draw() {
    if (scene_ != nullptr) {
        scene_->Draw();
    }
}