#pragma once
#include"MathFunction.h"
#include "DrawFunction.h"
#include"Object3D.h"
#include "Model.h"
#include "Camera.h"
#include "Rand.h"
#include <memory>

class Player;

class MoveEffect
{
private:

    // ワールド変換データ
    Transform transform_;

    // モデル
    Object3d* model_ = nullptr;

    // カメラ
    Camera* camera_ = nullptr;

    // 速度
    Vector3 velocity_ = {};
    float speedZ_ = 0.0f;

    // プレイヤーのポインタ
    Player* player = nullptr;

    // 行列
    Matrix4x4 worldMatrix_;

    // 移動範囲制限
    float moveRange_ = 5.0f;

    // ランダム用ポインタ
    Rand* rand_ = nullptr;
    int randomValue = 0;

public:

    // 初期化
    void Initialize(Object3d* model, Camera* camera, const Vector3& position,Player* player_);
    // 更新
    void Update();
    // 描画
    void Draw();
    // リセット
    void Reset();
    // コンストラクタとデストラクタ
    MoveEffect() = default;
    ~MoveEffect();
};

