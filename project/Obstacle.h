#pragma once
#include"MathFunction.h"
#include "DrawFunction.h"
#include"Object3D.h"
#include "Model.h"
#include "Camera.h"
#include <memory>

class Player;

class Obstacle
{
protected:
    // ワールド変換データ
    Transform transform_;

    // モデル
    Object3d* model_ = nullptr;

    // カメラ
    Camera* camera_ = nullptr;

    // 当たり判定サイズ
    float width;
    float height;

    // 死亡フラグ
    bool isDead_ = false;

public:


    Obstacle() = default;
    virtual ~Obstacle() = default;
    virtual void Initialize(Object3d* model, Camera* camera, const Vector3& position) = 0;
    virtual void Update();
    virtual void Draw();
    // ワールド座標の取得
    virtual Vector3 GetWorldPosition();
    // AABBを取得
    virtual AABB GetAABB();
    // 衝突応答
    virtual void OnCollision(const Player* player) = 0;
};

