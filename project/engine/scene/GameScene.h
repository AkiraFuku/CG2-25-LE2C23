#pragma once
#include"MathFunction.h"
#include "DrawFunction.h"
#include "Sprite.h"
#include"Object3D.h"
#include "Model.h"
#include "Camera.h"
#include "ParicleEmitter.h"
#include "Audio.h"
#include "TextureManager.h"
#include "Scene.h"
#include <memory>
class Player;
class ObstacleSlow;
class ObstacleNormal;
class ObstacleFast;
class ObstacleMax;
class MapChipField;

class GameScene :public Scene
{
public:
   
    void Initialize() override;
    void Finalize()override;
    void Update()override;
    void Draw()override;
    GameScene();
    ~GameScene() override;

    // 全ての当たり判定を行う
    void CheckAllCollisions();
    // 当たり判定
    bool isCollision(const AABB& aabb1, const AABB& aabb2);

    // マップチップの生成
    void GenerateFieldObjects();

private:
    std::unique_ptr<Camera> camera;
    std::unique_ptr<Sprite> sprite;
    std::unique_ptr<Object3d> object3d2;
    std::unique_ptr<Object3d> object3d;
    std::unique_ptr<ParicleEmitter> emitter;
    Audio::SoundHandle handle_;

    // 自キャラ
    std::unique_ptr<Player> player_;
    // プレイヤーのモデル
    std::unique_ptr<Object3d> playerModel_;

    // 障害物
    std::vector<std::unique_ptr<ObstacleSlow>> obstacleSlow_;
    std::vector<std::unique_ptr<ObstacleNormal>> obstacleNormal_;
    std::vector<std::unique_ptr<ObstacleFast>> obstacleFast_;
    std::vector<std::unique_ptr<ObstacleMax>> obstacleMax_;

    // 障害物のモデル
    std::vector<std::unique_ptr<Object3d>> obstacleSlowModel_;
    std::vector<std::unique_ptr<Object3d>> obstacleNormalModel_;
    std::vector<std::unique_ptr<Object3d>> obstacleFastModel_;
    std::vector<std::unique_ptr<Object3d>> obstacleMaxModel_;

    // マップチップフィールド
    std::unique_ptr<MapChipField> mapChipField_;
    // ブロック用のワールドトランスフォーム
    std::vector<std::vector<Transform*>> worldTransformObjects;

    //// 自キャラ
    //std::unique_ptr<Player> player_;
    //// プレイヤーのモデル
    //std::unique_ptr<Object3d> playerModel_;

    //// 障害物
    //std::vector<std::unique_ptr<ObstacleSlow>> obstacleSlow_;
    //std::vector<std::unique_ptr<ObstacleNormal>> obstacleNormal_;
    //std::vector<std::unique_ptr<ObstacleFast>> obstacleFast_;
    //std::vector<std::unique_ptr<ObstacleMax>> obstacleMax_;

    //// 障害物のモデル
    //std::vector<std::unique_ptr<Object3d>> obstacleSlowModel_;
    //std::vector<std::unique_ptr<Object3d>> obstacleNormalModel_;
    //std::vector<std::unique_ptr<Object3d>> obstacleFastModel_;
    //std::vector<std::unique_ptr<Object3d>> obstacleMaxModel_;

    //// マップチップフィールド
    //std::unique_ptr<MapChipField> mapChipField_;
    //// ブロック用のワールドトランスフォーム
    //std::vector<std::vector<Transform*>> worldTransformObjects;
};

