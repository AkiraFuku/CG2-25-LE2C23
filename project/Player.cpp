#include "Player.h"

void Player::Initialize(std::unique_ptr<Object3d>* model, std::unique_ptr<Camera>* camera, const Vector3& position)
{
    // nullポインタチェック
    assert(model);

    // 引数をメンバ変数に記録
    model_ = std::move(*model);
    camera_ = std::move(*camera);

}

void Player::Update()
{
    camera_->Update();
    model_->Update();
}

void Player::Draw()
{
    model_->Draw();
}
