#include "MoveEffect.h"
#include "Player.h"
#include "ModelManager.h"
#include <GameEngine.h>
#include <Framework.h>
#include <cassert>

void MoveEffect::Initialize(Object3d* model, Camera* camera, const Vector3& position, Player* player_)
{
    // nullポインタチェック
    assert(model);
    assert(camera);

    // 引数をメンバ変数に記録
    model_ = model;
    camera_ = camera;
    transform_.translate = position;
    player = player_;

    // ランダム値を決定
    rand_ = new Rand();
    rand_->Initialize();
    rand_->RandomInitialize();
    randomValue = static_cast<int>(rand_->GetRandom());

    // 位置を設定
    transform_.translate.x = player->GetWorldPosition().x + randomValue;
    // 角度を設定
    transform_.rotate = player->GetRotate();
    // 速度を設定
    speedZ_ = player->GetSpeedZ() * -1.0f;
    velocity_ = { 0.0f, 0.0f, speedZ_ };

    // 行列を設定
    worldMatrix_ = MakeAfineMatrix(transform_.scale, transform_.rotate, transform_.translate);

    // モデルの更新
    model_->SetTranslate(transform_.translate);
    model_->SetRotate(transform_.rotate);
    model_->Update();

}

void MoveEffect::Update()
{
    // 速度を初期化
    velocity_ = { 0.0f, 0.0f, speedZ_ };

    // 行列を更新
    worldMatrix_ = MakeAfineMatrix(transform_.scale, transform_.rotate, transform_.translate);

    // 速度を決定
    velocity_ = TransformNormal(velocity_, worldMatrix_);

    // 位置に速度を加算
    transform_.translate += velocity_;

    if (std::abs(transform_.translate.z - player->GetWorldPosition().z) > moveRange_)
    {
        // 移動範囲を超えたら戻す
        Reset();
    }

    // モデルの更新
    model_->SetTranslate(transform_.translate);
    model_->SetRotate(transform_.rotate);
    model_->Update();
}

void MoveEffect::Draw()
{
    model_->Draw();
}

void MoveEffect::Reset()
{
    // 位置を設定
    randomValue = static_cast<int>(rand_->GetRandom());
    transform_.translate.z = player->GetWorldPosition().z;
    transform_.translate.x = player->GetWorldPosition().x + randomValue;
    // 角度を設定
    transform_.rotate = player->GetRotate();
    // 速度を設定
    speedZ_ = player->GetSpeedZ() * -1.0f;
    velocity_ = { 0.0f, 0.0f, speedZ_ };

    worldMatrix_ = MakeAfineMatrix(transform_.scale, transform_.rotate, transform_.translate);
}

MoveEffect::~MoveEffect()
{
}
