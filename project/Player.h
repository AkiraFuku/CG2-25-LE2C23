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

    // 行列
    Matrix4x4 worldMatrix_;

    // 速度
    Vector3 velocity_ = {};
 
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
    // コンストラクタとデストラクタ
    Player() = default;
    ~Player();
};

