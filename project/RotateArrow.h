#pragma once
#include"MathFunction.h"
#include "DrawFunction.h"
#include"Object3D.h"
#include "Model.h"
#include "Camera.h"
#include <memory>

class Player;

class RotateArrow
{
private:
    // ワールド変換データ
    Transform transform_;

    // モデル
    Object3d* model_ = nullptr;

    // カメラ
    Camera* camera_ = nullptr;

    // プレイヤーのポインタ
    Player* player = nullptr;

    // 行列
    Matrix4x4 worldMatrix_;

    Vector3 targetPos_ = {};

public:
    // 初期化
    void Initialize(Object3d* model, Camera* camera, const Vector3& position, Player* player_);
    // 更新
    void Update();
    // 描画
    void Draw();
    // コンストラクタとデストラクタ
    RotateArrow() = default;
    ~RotateArrow();
};

