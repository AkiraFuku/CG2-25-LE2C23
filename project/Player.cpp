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
    velocity_ = { 0.0f, 0.0f, 0.05f };
    speedZ_ = velocity_.z;

}

void Player::Update()
{
    // 方向操作
    Rotate();

    if (isDriftStart_)
    {
        // ドリフト中の処理
        Drift();
    }
    else
    {
        if (Input::GetInstance()->TriggerKeyDown(DIK_SPACE))
        {
            // スペースキーを押したらドリフト開始
            isDriftStart_ = true;
            // 開始時の速度を記録
            preSpeedZ_ = speedZ_;
            // 開始時の角度を記録
            angleY_ = transform_.rotate.y;
        }
    }


    // 移動処理
    Move();

    // モデルの更新
    model_->SetTranslate(transform_.translate);
    model_->SetRotate(transform_.rotate);
    model_->Update();

    // カメラの更新
    MoveCamera();
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
    velocity_ = { 0.0f, 0.0f, speedZ_ };

    // 行列を更新
    worldMatrix_ = MakeAfineMatrix(transform_.scale, transform_.rotate, transform_.translate);

    // 速度を決定
    velocity_ = TransformNormal(velocity_, worldMatrix_);

    // 位置に速度を加算
    transform_.translate += velocity_;

}

void Player::MoveCamera()
{
    // 行列を更新
    worldMatrix_ = MakeAfineMatrix(transform_.scale, transform_.rotate, transform_.translate);

    // カメラの位置を更新
    Vector3 offset = { 0.0f, 5.0f, -20.0f };
    offset = TransformNormal(offset, worldMatrix_);
    targetPos_ = transform_.translate + offset;
    cameraTransform_.translate = Lerp(cameraTransform_.translate, targetPos_, kInterpolationRate);
    camera_->SetTranslate(cameraTransform_.translate);
    cameraTransform_.rotate.y = transform_.rotate.y;
    camera_->SetRotate(cameraTransform_.rotate);
}

void Player::Drift()
{
    if (isAcceleration_)
    {
        if (speedZ_ >= topSpeedZ_)
        {
            // 最大速度に達したら加速終了
            isAcceleration_ = false;
            isDriftStart_ = false;
            driftTimer_ = 0.0f;
        }
        else
        {
            speedZ_ += kAcceleration;
        }

    }
    else
    {
        if (Input::GetInstance()->TriggerKeyUp(DIK_SPACE))
        {
            // スペースキーを離したら加速開始
            isAcceleration_ = true;
            // どれくらい曲がったかを記録
            driftAngle_ = std::abs(transform_.rotate.y - angleY_);
            // 最大速度を決定
            topSpeedZ_ = preSpeedZ_ + (driftAngle_ * driftTimer_);

            if (topSpeedZ_ > kMaxSpeedZ)
            {
                // 最大速度を越さないよう調整
                topSpeedZ_ = kMaxSpeedZ;
            }
        }
        else
        {
            // 減速
            speedZ_ *= kSpeedDecayRate;
            // タイマーを加算
            driftTimer_ += 0.1f;
        }

    }

}

Player::~Player()
{
    
}
