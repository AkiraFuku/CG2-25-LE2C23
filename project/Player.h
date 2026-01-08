#pragma once
#include <GameEngine.h>
#include <Framework.h>
#include <memory>
#include"MathFunction.h"
#include"Object3D.h"
#include "Model.h"
#include "Camera.h"


class Player
{
private:
    
    // モデル
    std::unique_ptr<Object3d> model_;

    // カメラ
    std::unique_ptr<Camera> camera_;

    // 速度
    Vector3 velocity_ = {};

public:
    void Initialize(std::unique_ptr<Object3d>* model, std::unique_ptr<Camera>* camera, const Vector3& position);
    void Update();
    void Draw();
    const Vector3& GetVelocity() const { return velocity_; }
};

