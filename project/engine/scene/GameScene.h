#pragma once

#include "Audio.h"
#include "Camera.h"
#include "CourseWall.h"
#include "MathFunction.h"
#include "Model.h"
#include "Object3D.h"
#include "ParicleEmitter.h"
#include "Scene.h"
#include "Sprite.h"
#include "TextureManager.h"

#include <memory>
#include <vector>

class GameScene : public Scene {
public:
  void Initialize() override;
  void Finalize() override;
  void Update() override;
  void Draw() override;

private:
  // =============================
  // コース壁（描画 + 当たり判定）
  // =============================
  // ※ CourseWall にクラス化
  std::unique_ptr<CourseWall> courseWall_;

  // 壁生成（中心線を作って courseWall_ に渡す）
  void BuildCourseWalls();

private:
  // テスト用の「車」(球当たり判定 + Object3d描画)
  struct Car {
    std::unique_ptr<Object3d> object;
    Vector3 pos{0.0f, 0.5f, 0.0f};
    Vector3 vel{0.0f, 0.0f, 0.0f};
    float radius = 0.35f; // 壁との当たり判定半径
  };

  // 既存
  std::unique_ptr<Camera> camera;
  std::unique_ptr<Sprite> sprite;
  std::unique_ptr<Object3d> object3d2;
  std::unique_ptr<Object3d> object3d;
  std::unique_ptr<ParicleEmitter> emitter;
  Audio::SoundData soundData1;

  // 追加
  Car car_{};

  // コースパラメータ（ImGuiで後から弄る用にも）
  float courseHalfWidth_ = 5.0f; // コース中心線から左右壁までの距離
  float wallHeight_ = 1.2f;      // 壁の高さ
  float wallThickness_ = 3.5f;   // 壁の厚み（当たり判定に使う）
  float carMoveSpeed_ = 6.0f;    // 移動速度（60fps想定の簡易）
};
