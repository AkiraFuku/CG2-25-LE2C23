#pragma once
#include "Camera.h"
#include "MathFunction.h" // Vector3などの定義
#include <memory>

// 前方宣言 (Playerクラスのヘッダをインクルードせずにポインタだけ使う)
class Player;

class CameraController {
public:
    // 移動範囲制限用の構造体
    struct Rect {
        float left = -100.0f;
        float right = 100.0f;
        float bottom = -100.0f;
        float top = 100.0f;
    };

    void Initialize(Camera* camera);
    void Update();
    void Reset();

    // 追従対象のセット
    void SetTarget(Player* target) { target_ = target; }
    
    // 移動範囲のセット
    void SetMoveArea(const Rect& area) { moveArea_ = area; }

private:
    Camera* camera_ = nullptr;
    Player* target_ = nullptr;

    Rect moveArea_;
    Vector3 targetOffset_ = { 0.0f, 0.0f, -15.0f };
    Vector3 destination_ = { 0.0f, 0.0f, 0.0f };

    static inline const float kInterpolationRate = 0.1f;
    static inline const float kVelocityBias = 30.0f;

    // ターゲットが画面端のどこまで行ったらカメラが動くか（マージン）
    struct Margin {
        float left = -9.0f;
        float right = 9.0f;
        float bottom = -5.0f;
        float top = 5.0f;
    };
    Margin targetMargin_;
};
