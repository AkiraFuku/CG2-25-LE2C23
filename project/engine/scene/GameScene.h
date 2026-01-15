#pragma once
#include "Audio.h"
#include "Camera.h"
#include "DrawFunction.h"
#include "MathFunction.h"
#include "Model.h"
#include "Object3D.h"
#include "ParicleEmitter.h"
#include "Scene.h"
#include "Sprite.h"
#include "TextureManager.h"
#include <string>
#include <vector>

#include <memory>
class Player;
class ObstacleSlow;

#include "CourseWall.h"

class GameScene : public Scene {
public:
  void Initialize() override;
  void Finalize() override;
  void Update() override;
  void Draw() override;
  GameScene();
  ~GameScene() override;

  // 全ての当たり判定を行う
  void CheckAllCollisions();
  // 当たり判定
  bool isCollision(const AABB &aabb1, const AABB &aabb2);

bool LoadCourseFromCommandCsv(const std::string &csvPath,
                                const std::string &wallModelName = "cube.obj");


private:
  std::unique_ptr<Camera> camera;
  std::unique_ptr<Sprite> sprite;
  std::unique_ptr<Object3d> object3d2;
  std::unique_ptr<Object3d> object3d;
  std::unique_ptr<ParicleEmitter> emitter;
  Audio::SoundData soundData1;

  // 自キャラ
  std::unique_ptr<Player> player_;
  // プレイヤーのモデル
  std::unique_ptr<Object3d> playerModel_;

  // 障害物
  std::unique_ptr<ObstacleSlow> obstacleSlow_;
  // 障害物のモデル
  std::unique_ptr<Object3d> obstacleSlowModel_;

  std::vector<std::unique_ptr<Object3d>> wallObj_;
  std::vector<CourseWall> walls_;
};
