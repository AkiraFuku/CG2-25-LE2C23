#pragma once
#include"MathFunction.h"
#include "DrawFunction.h"
#include"Object3D.h"
#include "Model.h"
#include "Camera.h"
#include <memory>

class Goal
{
private:

    // ワールド変換データ
    Transform transform_;

    // モデル
    Object3d* model_ = nullptr;

    // カメラ
    Camera* camera_ = nullptr;

    // 当たり判定サイズ
    static inline float kWidth = 2.0f;
    static inline float kHeight = 2.0f;

public:

    // 初期化
    void Initialize(Object3d* model, Camera* camera, const Vector3& position);
    // 更新
    void Update();
    // 描画
    void Draw();
    // ワールド座標の取得
    Vector3 GetWorldPosition();
    // AABBを取得
    AABB GetAABB();
};

