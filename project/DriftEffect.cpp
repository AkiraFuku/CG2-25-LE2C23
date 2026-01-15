#include "DriftEffect.h"
#include "Player.h"
#include "ModelManager.h"
#include <GameEngine.h>
#include <Framework.h>


void DriftEffect::Initialize(Sprite* sprite, Camera* camera, const Vector3& position, Player* player_)
{
    // nullポインタチェック
    assert(sprite);
    assert(camera);

    // 引数をメンバ変数に記録
    sprite_ = sprite;
    camera_ = camera;
    transform_.translate = position;
    player = player_;

}

void DriftEffect::Update()
{

    if (player->IsAcceleration())
    {
        // tの値を増加
        t += 0.02f;

        if (!isAlphaMax_)
        {
            if (color_.w >= 1.0f) {
                // ターゲットアルファ値を0に設定
                targetAlpha_ = 0.0f;
                // tをリセット
                t = 0.0f;
                // フラグを立てる
                isAlphaMax_ = true;
            }

        }

    }

    // モデルのアルファ値をイージング
    color_.w = EaseOutFloat(t, color_.w, targetAlpha_);
    sprite_->SetColor(color_);
    sprite_->Update();
}

void DriftEffect::Draw()
{
    sprite_->Draw();
}

void DriftEffect::ResetAlpha()
{
    t = 0.00f;
    color_.w = 0.0f;
    targetAlpha_ = 1.0f;
    isAlphaMax_ = false;

}

DriftEffect::~DriftEffect()
{
}
