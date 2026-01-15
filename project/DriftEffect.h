#pragma once
#include"MathFunction.h"
#include "DrawFunction.h"
#include "Sprite.h"
#include "Camera.h"
#include <memory>

class Player;

class DriftEffect
{
private:

    // ワールド変換データ
    Transform transform_;

    // スプライト
    Sprite* sprite_ = nullptr;

    // カメラ
    Camera* camera_ = nullptr;

    // プレイヤーのポインタ
    Player* player = nullptr;

    // モデルのアルファ値
    float targetAlpha_ = 1.0f;
    Vector4 color_ = { 1.0f,1.0f,1.0f,0.0f };

    // イージング用変数
    float t = 0.0f;

    // アルファ値が上がり切ったかどうかのフラグ
    bool isAlphaMax_ = false;

public:
    // 初期化
    void Initialize(Sprite* sprite, Camera* camera, const Vector3& position, Player* player_);
    // 更新
    void Update();
    // 描画
    void Draw();
    // アルファ値をリセット
    void ResetAlpha();
    // コンストラクタとデストラクタ
    DriftEffect() = default;
    ~DriftEffect();
};

