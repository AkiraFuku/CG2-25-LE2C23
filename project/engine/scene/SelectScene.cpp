#include "SelectScene.h"
#include "Camera.h"
#include "Fade.h"
#include "Input.h"
#include "SceneManager.h"
#include "imgui.h"
#include "GameContext.h"

void SelectScene::Initialize() {

  camera_ = std::make_unique<Camera>();
  camera_->SetRotate({0.0f, 0.0f, 0.0f});
  camera_->SetTranslate({0.0f, 0.0f, -5.0f});

  fade_ = std::make_unique<Fade>();
  fade_->Initialize(camera_.get());
  fade_->Start(Fade::Phase::kFadeIn);

  selection_ = Selection::kTutorial;
}

void SelectScene::Finalize() {}

void SelectScene::Update() {
  // 前回の入力を保存しておき、変化があったときだけ音を鳴らすなどの判定に使う
  int prevSelection = currentSelectionIndex_;

  // --- 入力処理（上・下） ---
  // 下キー（またはS）を押した
  if (Input::GetInstance()->TriggerKeyDown(DIK_S) ||
      Input::GetInstance()->TriggerPadDown(0, XINPUT_GAMEPAD_DPAD_DOWN)) {
    currentSelectionIndex_++;
  }
  // 上キー（またはW）を押した
  if (Input::GetInstance()->TriggerKeyDown(DIK_W) ||
      Input::GetInstance()->TriggerPadDown(0, XINPUT_GAMEPAD_DPAD_UP)) {
    currentSelectionIndex_--;
  }

  // --- ループ処理（数学的にきれいな書き方） ---
  // これにより、範囲外に出ても自動的に反対側に回ります
  // 例: 2になったら0に戻る、-1になったら1になる
  int count = static_cast<int>(Selection::kNone);
  currentSelectionIndex_ = (currentSelectionIndex_ + count) % count;

  // --- 選択変更時の処理 ---
  if (prevSelection != currentSelectionIndex_) {
    // ここで「カッ」というSEを鳴らすと気持ちいい
    // Audio::GetInstance()->PlayWave(seSelect_);
  }

  // --- 決定処理 ---
  if (Input::GetInstance()->TriggerKeyDown(DIK_SPACE) ||
      Input::GetInstance()->TriggerPadDown(0, XINPUT_GAMEPAD_A)) {

    if (!requestSceneChange_) {
      // 決定音を鳴らす
      // Audio::GetInstance()->PlayWave(seDecide_);

      // ★ここで「次のシーン名」をセットする
      switch (static_cast<Selection>(currentSelectionIndex_)) {
      case Selection::kTutorial:
        nextSceneName_ = "TutorialScene";
        break;

      case Selection::kStage1:
        GameContext::GetInstance()->SetStageNum(1); // データ書き込み
        nextSceneName_ = "GameScene";
        break;

      case Selection::kStage2:
        GameContext::GetInstance()->SetStageNum(2);
        nextSceneName_ = "GameScene";
        break;

      case Selection::kStage3:
        GameContext::GetInstance()->SetStageNum(3);
        nextSceneName_ = "GameScene";
        break;
      }

      // フェードアウト開始
      fade_->Start(Fade::Phase::kFadeOut);
      requestSceneChange_ = true;
    }
  }

  // 画面を覆い切った瞬間にシーン切替
  if (requestSceneChange_ && fade_->IsCovering()) {
    // ★保存しておいたシーン名を使って遷移する
    GetSceneManager()->ChangeScene(nextSceneName_);
    return;
  }

  camera_->Update();
  fade_->Update();

#ifdef USE_IMGUI
  ImGui::Begin("Debug");

  ImGui::Text("Scene Selection Debug");
  ImGui::Separator(); // 線の描画

  // 生のインデックス番号を表示（デバッグ用）
  ImGui::Text("Index: %d", currentSelectionIndex_);

  // インデックスに応じたシーン名を表示
  // 文字色を変えるとより分かりやすいです
  ImGui::Text("Current Target: ");
  ImGui::SameLine(); // 改行せず横に並べる

  switch (static_cast<Selection>(currentSelectionIndex_)) {
  case Selection::kTutorial:
    ImGui::TextColored(ImVec4(0, 1, 0, 1), "Tutorial Scene"); // 緑色で表示
    break;
  case Selection::kStage1:
    ImGui::TextColored(ImVec4(0, 1, 1, 1), "Stage1"); // 水色で表示
    break;
  case Selection::kStage2:
    ImGui::TextColored(ImVec4(0, 1, 1, 1), "Stage2"); // 水色で表示
    break;
  case Selection::kStage3:
    ImGui::TextColored(ImVec4(0, 1, 1, 1), "Stage3"); // 水色で表示
    break;
  default:
    ImGui::TextColored(ImVec4(1, 0, 0, 1), "Unknown"); // 赤色
    break;
  }

  // 遷移リクエスト中かどうかも表示
  if (requestSceneChange_) {
    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Transiting to: %s...",
                       nextSceneName_.c_str());
  }

  ImGui::End();
#endif // USE_IMGUI
}

void SelectScene::Draw() { fade_->Draw(); }
