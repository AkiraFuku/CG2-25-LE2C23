#include "RotateArrow.h"
#include "Player.h"
#include "ModelManager.h"
#include <GameEngine.h>
#include <Framework.h>


void RotateArrow::Initialize(Object3d* model, Camera* camera, const Vector3& position, Player* player_)
{
    // nullポインタチェック
    assert(model);
    assert(camera);

    // 引数をメンバ変数に記録
    model_ = model;
    camera_ = camera;
    transform_.translate = position;
    player = player_;
}

void RotateArrow::Update()
{
    // 行列を更新
    worldMatrix_ = player->GetWorldMatrix();

    // 位置を更新
    Vector3 offset = { 0.0f, 0.0f, 4.0f };
    offset = TransformNormal(offset, worldMatrix_);
    targetPos_ = player->GetWorldPosition() + offset;
    transform_.translate = targetPos_;
    
    // モデルを更新
    model_->SetTranslate(transform_.translate);
    model_->SetRotate(player->GetRotate());
    model_->Update();
}

void RotateArrow::Draw()
{
    // 矢印の描画
    model_->Draw();
}

RotateArrow::~RotateArrow()
{
}
