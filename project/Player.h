#pragma once
#include"MathFunction.h"
#include "DrawFunction.h"
#include"Object3D.h"
#include "Model.h"
#include "Sprite.h"
#include "Camera.h"
#include <memory>

class ObstacleSlow;
class ObstacleNormal;
class ObstacleFast;
class ObstacleMax;
class MapChipField;
class MoveEffect;
class DriftEffect;
class RotateArrow;
class SpeedMeter;

// 速さの段階
enum class SpeedStage
{
    kSlow,
    kNormal,
    kFast,
    kMax
};

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

    // 現在の速度段階
    SpeedStage currentSpeedStage_ = SpeedStage::kSlow;
    const float kSlowSpeedZ = 0.2f;
    const float kNormalSpeedZ = 0.5f;
    const float kFastSpeedZ = 0.8f;

    // 当たり判定サイズ
    static inline float kWidth = 2.0f;
    static inline float kHeight = 2.0f;

    // 死亡フラグ
    bool isDead_ = false;

    // マップチップのフィールド
    MapChipField* mapChipField_ = nullptr;

    // 移動エフェクト
    std::vector<std::unique_ptr<MoveEffect>> moveEffects_;
    std::vector<std::unique_ptr<Object3d>> moveEffectModels_;
    static inline const uint32_t kNumMoveEffects = 10;

    // 加速エフェクト
    std::unique_ptr<DriftEffect> driftEffect_;
    std::unique_ptr<Sprite> driftEffectSprite_;

    // 方向矢印
    std::unique_ptr<RotateArrow> rotateArrow_;
    std::unique_ptr<Object3d> rotateArrowModel_;

    // スピードメーター
    std::unique_ptr<SpeedMeter> speedMeter_;
    std::unique_ptr<Sprite> speedMeterSprite_;
    std::unique_ptr<Sprite> baseSprite_;

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
    // 速度の段階を決定
    void DetermineSpeedStage();
    // 速度の段階を取得
    SpeedStage GetSpeedStage() const { return currentSpeedStage_; }
    // 速度Zを取得
    float GetSpeedZ() const { return speedZ_; }
    // ワールド座標の取得
    Vector3 GetWorldPosition();
    // AABBを取得
    AABB GetAABB();
    // 衝突応答
    void OnCollision(const ObstacleSlow* obstacleSlow);
    void OnCollision(const ObstacleNormal* obstacleNormal);
    void OnCollision(const ObstacleFast* obstacleFast);
    void OnCollision(const ObstacleMax* obstacleMax);
    // デスフラグを取得
    bool IsDead() const { return isDead_; }
    // マップチップフィールドのセット
    void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }
    // 角度を取得
    Vector3 GetRotate() const { return transform_.rotate; }
    // 加速フラグを取得
    bool IsAcceleration() const { return isAcceleration_; }
    // 行列を取得
    Matrix4x4 GetWorldMatrix() const { return worldMatrix_; }
    // コンストラクタとデストラクタ
    Player();
    ~Player();
};

