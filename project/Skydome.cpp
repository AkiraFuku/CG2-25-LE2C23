#include "Skydome.h"
#include "ModelManager.h"
void SkyDome::Initialize() {
    // 3Dオブジェクトのインスタンス生成
    object_ = std::make_unique<Object3d>();
    object_->Initialize();

    ModelManager::GetInstance()->LoadModel("skydome/Skydome.obj");
    // モデルのセット (パスは適宜自作エンジンのルールに合わせてください)
    object_->SetModel("skydome/Skydome.obj"); 



    // 必要なら大きさを調整
    
    object_->SetRadius(1000.0f);
}

void SkyDome::Update() {
    // 必要な更新処理があればここへ（回転させるなど）
    object_->Update();
}

void SkyDome::Draw() {
    object_->Draw();
}