#include "BubbleFade.h"

#include "BubbleParticle.h"
#include "Camera.h"
#include "DXCommon.h"

#include <algorithm>

void BubbleFade::Initialize(Camera *camera,
                            const std::string &bubbleTexturePath,
                            const Params &params) {
  camera_ = camera;
  params_ = params;
  texturePath_ = bubbleTexturePath;

  mode_ = Mode::kNone;
  timer_ = 0.0f;
  emitAccum_ = 0.0f;
  coverComplete_ = false;
  finished_ = false;

  bubble_ = BubbleParticle::GetInstance();
  bubble_
      ->Initialize(); // 何回呼んでもOKにしたいなら、BubbleParticle側で二重Initialize防止しても良い
  bubble_->SetCamera(camera_);

  // グループは Fade が作る（TitleSceneから借りない）
  bubble_->CreateGroup(groupName_, texturePath_);
  bubbleReady_ = true;
}

void BubbleFade::Finalize() {
  if (bubbleReady_) {
    bubble_->ReleaseGroup(groupName_);
    bubbleReady_ = false;
  }
  // BubbleParticle自体のFinalize()は「全シーンで使い終わった時」に呼ぶ方が安全
  // bubble_->Finalize();
}

void BubbleFade::StartFadeOut() {
  if (!bubbleReady_)
    return;

  mode_ = Mode::kFadeOut;
  timer_ = 0.0f;
  emitAccum_ = 0.0f;
  coverComplete_ = false;
  finished_ = false;
}

void BubbleFade::StartFadeIn() {
  if (!bubbleReady_)
    return;

  mode_ = Mode::kFadeIn;
  timer_ = 0.0f;
  emitAccum_ = 0.0f;
  coverComplete_ = false;
  finished_ = false;

  if (params_.clearOnFadeInStart) {
    // 泡を消したいなら：グループ作り直しが一番安全（listを触らなくて済む）
    bubble_->ReleaseGroup(groupName_);
    bubble_->CreateGroup(groupName_, texturePath_);
  }
}

void BubbleFade::Update() {
  if (!bubbleReady_)
    return;
  if (mode_ == Mode::kNone)
    return;

  const float dt = DXCommon::kDeltaTime;

  switch (mode_) {
  case Mode::kFadeOut:
    UpdateFadeOut(dt);
    break;
  case Mode::kFadeIn:
    UpdateFadeIn(dt);
    break;
  default:
    break;
  }

  // 既存の泡は常に動かす
  bubble_->Update();
}

void BubbleFade::UpdateFadeOut(float dt) {
  timer_ += dt;

  // FadeOut中は生成し続ける
  emitAccum_ += params_.emitPerSec * dt;

  // basePosを中心に、X/Zを広く散らして「画面全体っぽく」する
  // 位置自体は BubbleParticle::Emit が offX/offZ を持ってるので、
  // ここでは emitterBasePos を「画面下っぽい位置」にしておけばOK。
  const Vector3 basePos = params_.emitterBasePos;

  // 呼び出し回数を減らす（emitBatch個ずつ吐く）
  while (emitAccum_ >= static_cast<float>(params_.emitBatch)) {
    emitAccum_ -= static_cast<float>(params_.emitBatch);

    // BubbleParticle側の offX/offZ だけでも広がるけど、
    // Fade側でも少し散らしたいならここで basePos を揺らす手もある。
    // （今は basePos 固定でOK）
    bubble_->Emit(groupName_, basePos, params_.emitBatch);
  }

  // 覆い完了（時間で判定）
  if (timer_ >= params_.fadeOutTime) {
    coverComplete_ = true;
    // この瞬間に Scene切替したいなら、外側で IsCoverComplete() を見て切り替える
  }
}

void BubbleFade::UpdateFadeIn(float dt) {
  timer_ += dt;

  // FadeIn中は生成しない（泡は Update() の bubble_->Update() で勝手に消える）
  // 終了判定
  if (timer_ >= params_.fadeInTime) {
    mode_ = Mode::kNone;
    finished_ = true;
  }
}

void BubbleFade::Draw() {
  if (!bubbleReady_)
    return;
  if (mode_ == Mode::kNone)
    return;

  bubble_->Draw();
}
