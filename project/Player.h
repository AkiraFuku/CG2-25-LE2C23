#pragma once
#include"MathFunction.h"
#include"Object3D.h"
#include "Model.h"
#include "Camera.h"
#include <memory>


class Player
{
private:
    
    // ワールド変換データ
    Transform transform_;

    // モデル
    Object3d* model_ = nullptr;

    // カメラ
    Camera* camera_ = nullptr;
    Transform cameraTransform_;
    Vector3 targetPos_ = {};

    // 座標補完割合
    static inline const float kInterpolationRate = 0.1f;

    // 行列
    Matrix4x4 worldMatrix_;

    // 角度
    float driftAngle_ = 0.0f;
    float angleY_ = 0.0f;

    // 速度
    Vector3 velocity_ = {};
    float speedZ_ = 0.0f;
    float preSpeedZ_ = 0.0f;

    // 速度減衰率
    const float kSpeedDecayRate = 0.98f;
    float driftTimer_ = 0.0f;

    // 加速度
    const float kAcceleration = 0.02f;
    float topSpeedZ_ = 0.0f;

    // 最大速度
    const float kMaxSpeedZ = 1.0f;

    // ドリフト開始フラグ
    bool isDriftStart_ = false;

    // 加速フラグ
    bool isAcceleration_ = false;
 
public:
    // 初期化
    void Initialize(Object3d* model, Camera* camera, const Vector3& position);
    // 更新
    void Update();
    // 描画
    void Draw();
    // 旋回
    void Rotate();
    // 移動
    void Move();
    // カメラ移動
    void MoveCamera();
    // ドリフト
    void Drift();
    // コンストラクタとデストラクタ
    Player() = default;
    ~Player();
};

