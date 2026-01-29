#pragma once

#include <memory>
#include <string>

#include "Scene.h"

class Fade;
class Camera;

class SelectScene : public Scene {
public:
  enum class Selection {
    kTutorial, // チュートリアル
    kStage1,
    kStage2,
    kStage3,
    kNone,     // 選択なし
  };

  Selection selection_ = Selection::kNone;

public:
  void Initialize() override;
  void Finalize() override;
  void Update() override;
  void Draw() override;

private:
  int currentSelectionIndex_ = 0;

  // 次に遷移するシーン名を保存しておく変数
  std::string nextSceneName_ = "";

  std::unique_ptr<Camera> camera_;
  std::unique_ptr<Fade> fade_;

  bool requestSceneChange_ = false;
};
