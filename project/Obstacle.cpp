#include "Obstacle.h"
#include "ModelManager.h"
#include <GameEngine.h>
#include <Framework.h>


void Obstacle::Update()
{
    if (isDead_)
    {
        return;
    }

    // モデルの更新
    model_->SetTranslate(transform_.translate);
    model_->Update();
    // カメラの更新
    camera_->Update();
}

void Obstacle::Draw()
{
    if (isDead_)
    {
        return;
    }

    model_->Draw();
}

Vector3 Obstacle::GetWorldPosition()
{
    // ワールド座標を入れる変数
    Vector3 worldPos;
    // ワールド行列の平行移動成分を取得
    worldPos = transform_.translate;
    return worldPos;
}

AABB Obstacle::GetAABB()
{
    Vector3 worldPos = GetWorldPosition();
    AABB aabb;

    aabb.min = { worldPos.x - width / 2.0f, worldPos.y - height / 2.0f, worldPos.z - width / 2.0f };
    aabb.max = { worldPos.x + width / 2.0f, worldPos.y + height / 2.0f, worldPos.z + width / 2.0f };

    return aabb;
}
