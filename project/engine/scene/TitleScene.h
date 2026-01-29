#pragma once
#include "Audio.h"
#include "Camera.h"
#include "MathFunction.h"
#include "Model.h"
#include "Object3D.h"
#include "ParicleEmitter.h"
#include "Sprite.h"
#include "TextureManager.h"

#include "Fade.h"

#include "Scene.h"
#include <memory>
class TitleScene : public Scene {
public:
  enum class Phase {
    kIdle,
    kFadeIn, // フェードイン
    kFadOut, // フェードアウト
  };

  Phase phase_ = Phase::kIdle;

public:
  void Initialize() override;
  void Finalize() override;
  void Update() override;
  void Draw() override;

private:
  std::unique_ptr<Camera> camera;
  std::unique_ptr<Sprite> sprite;

  uint32_t handle_ = 0;

  // メンバ
  std::unique_ptr<Fade> fade_;
  bool requestSceneChange_ = false;
};
