#include "ObstacleSlow.h"
#include "Player.h"

void ObstacleSlow::Initialize(Object3d* model, Camera* camera, const Vector3& position)
{
    // nullポインタチェック
    assert(model);
    assert(camera);

    // 引数をメンバ変数に記録
    model_ = model;
    camera_ = camera;
    transform_.translate = position;

    // 当たり判定サイズを調整
    width = 2.0f;
    height = 2.0f;
}

void ObstacleSlow::OnCollision(const Player* player)
{
    // プレイヤーの速度が一定以上なら破壊
    if (player->GetSpeedStage() == SpeedStage::kSlow)
    {
        isDead_ = true;
    }
}
