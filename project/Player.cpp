#include "Player.h"
#include "ModelManager.h"
#include "Input.h"
#include <GameEngine.h>
#include <Framework.h>

void Player::Initialize(Object3d* model, Camera* camera, const Vector3& position)
{
    // nullポインタチェック
    assert(model);
    assert(camera);

    // 引数をメンバ変数に記録
    model_ = model;
    camera_ = camera;
    cameraTransform_.rotate = camera->GetRotate();
    cameraTransform_.translate = camera->GetTranslate();
    transform_ = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
    transform_.translate = position;
    worldMatrix_ = MakeAfineMatrix(transform_.scale, transform_.rotate, transform_.translate);

    // 速度を初期化
    velocity_ = { 0.0f, 0.02f, 0.0f };

}

void Player::Update()
{
    // 方向操作
    Rotate();

    // 移動処理
    Move();

    // モデルの更新
    model_->SetTranslate(transform_.translate);
    model_->SetRotate(transform_.rotate);
    model_->Update();

    // カメラの更新
    camera_->Update();

}

void Player::Draw()
{
    model_->Draw();
}

void Player::Rotate()
{
    // 回転速さ[ラジアン/frame]
    const float kRotSpeed = 0.01f;

    // 押した方向で移動ベクトルを変更
    if (Input::GetInstance()->PushedKeyDown(DIK_A)) {
        transform_.rotate.y += kRotSpeed;
    }
    else if (Input::GetInstance()->PushedKeyDown(DIK_D)) {
        transform_.rotate.y -= kRotSpeed;
    }
}

void Player::Move()
{
    // 速度を初期化
    velocity_ = { 0.0f, 0.0f, 0.02f };

    // 行列を更新
    worldMatrix_ = MakeAfineMatrix(transform_.scale, transform_.rotate, transform_.translate);

    // 速度を決定
    velocity_ = TransformNormal(velocity_, worldMatrix_);

    // 位置に速度を加算
    transform_.translate += velocity_;

    // カメラの位置を更新
    Vector3 offset = { 0.0f, 5.0f, -20.0f };
    offset = TransformNormal(offset, worldMatrix_);
    cameraTransform_.translate = transform_.translate + offset;
    camera_->SetTranslate(cameraTransform_.translate);
    cameraTransform_.rotate.y = transform_.rotate.y;
    camera_->SetRotate(cameraTransform_.rotate);
}

Player::~Player()
{
    
}
