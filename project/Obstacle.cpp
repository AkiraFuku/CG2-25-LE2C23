#include "Obstacle.h"
#include "Score.h"
#include "Player.h"
#include "ModelManager.h"
#include <GameEngine.h>
#include <Framework.h>
#include <cassert>

Obstacle::Obstacle() = default;

Obstacle::~Obstacle()
{
    // スコアの解放
    scores_.clear();
    scoreModels_.clear();
}

void Obstacle::Update()
{
    if (isDead_)
    {

        // デスフラグの立ったスコアを削除
        scores_.erase(std::remove_if(scores_.begin(), scores_.end(),
            [](const std::unique_ptr<Score>& score) { return score->IsDead(); }),
            scores_.end());

        // スコアの更新
        for (auto& score : scores_)
        {
            if (score->IsDead())
            {
                continue;
            }

            score->Update();
        }

    }
    else
    {
        // モデルの更新
        model_->SetTranslate(transform_.translate);
        model_->Update();
    }

    CheckCollision();
   
}

void Obstacle::Draw()
{
    if (isDead_)
    {
        // スコアの描画
        for (auto& score : scores_)
        {
            if (score->IsDead())
            {
                continue;
            }

            score->Draw();
        }
       
    }
    else
    {
        // 障害物の描画
        model_->Draw();
    }

    
}

Vector3 Obstacle::GetWorldPosition() const
{
    // ワールド座標を入れる変数
    Vector3 worldPos;
    // ワールド行列の平行移動成分を取得
    worldPos = transform_.translate;
    return worldPos;
}

AABB Obstacle::GetAABB() const
{
    Vector3 worldPos = GetWorldPosition();
    AABB aabb;

    aabb.min = { worldPos.x - width / 2.0f, worldPos.y - height / 2.0f, worldPos.z - width / 2.0f };
    aabb.max = { worldPos.x + width / 2.0f, worldPos.y + height / 2.0f, worldPos.z + width / 2.0f };

    return aabb;
}

bool Obstacle::isCollision(const AABB& aabb1, const AABB& aabb2)
{
    if (aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x && aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y && aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z) {
        return true;
    }

    return false;
}

void Obstacle::CheckCollision()
{
    // 判定対象1と2の座標
    AABB aabbPlayer, aabbScore;
    // プレイヤーの座標
    aabbPlayer = player_->GetAABB();
    // スコアの座標
    for (auto& score : scores_) {
        aabbScore = score->GetAABB();

        if (score->IsDead())
        {
            continue;
        }

        // 当たり判定
        if (isCollision(aabbPlayer, aabbScore))
        {
            
            // 衝突応答
            score->OnCollision();
        }
    }
}

bool Obstacle::IsScoreNone() const
{
    // 本体が壊れているかつ管理下のスコアが空
    return isDead_ && scores_.empty();
}
