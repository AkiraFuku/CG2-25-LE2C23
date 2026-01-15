#include "Goal.h"
#include "ModelManager.h"
#include <GameEngine.h>
#include <Framework.h>
#include <cassert>


void Goal::Initialize(Object3d* model, Camera* camera, const Vector3& position)
{
    // nullポインタチェック
    assert(model);
    assert(camera);

    // 引数をメンバ変数に記録
    model_ = model;
    camera_ = camera;
    transform_.translate = position;

    // モデルの更新
    model_->SetTranslate(transform_.translate);
    model_->Update();
}

void Goal::Update()
{
    // モデルの更新
    model_->SetTranslate(transform_.translate);
    model_->Update();
}

void Goal::Draw()
{
    // モデルの描画
    model_->Draw();
}

Vector3 Goal::GetWorldPosition()
{
    // ワールド座標を入れる変数
    Vector3 worldPos;
    // ワールド行列の平行移動成分を取得
    worldPos = transform_.translate;
    return worldPos;
}

AABB Goal::GetAABB()
{
    Vector3 worldPos = GetWorldPosition();
    AABB aabb;

    aabb.min = { worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f };
    aabb.max = { worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f };

    return aabb;
}
