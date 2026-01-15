#pragma once
#include"MathFunction.h"
#include "DrawFunction.h"
#include "Sprite.h"
#include "Camera.h"
#include <memory>

class Player;

class SpeedMeter
{
private:
    // ワールド変換データ
    Transform transform_;

    // スプライト
    Sprite* sprite_ = nullptr;
    Sprite* baseSprite_ = nullptr;

    // カメラ
    Camera* camera_ = nullptr;

    // プレイヤーのポインタ
    Player* player = nullptr;

public:
    // 初期化
    void Initialize(Sprite* sprite, Sprite* baseSprite, Camera* camera,Player* player_);
    // 更新
    void Update();
    // 描画
    void Draw();
    // コンストラクタとデストラクタ
    SpeedMeter() = default;
    ~SpeedMeter();
};

