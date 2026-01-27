//====================================================
// Fade.h
//====================================================
#pragma once

class BubbleParticle;
class Camera;

#include "Object3d.h"
#include <cstdint>
#include <memory>

class Fade {
public:
  enum class Phase {
    kFadeIn,
    kIdle,
    kFadeOut,
    kSceneTransition,
  };

public:
  void Initialize(Camera *camera);
  void Update();
  void Draw();
  void Start(Phase phase);

  bool IsFinished() const { return finished_; }
  bool IsCovering() const { return covering_; }

private:
  Phase phase_ = Phase::kIdle;

  BubbleParticle *bubble_ = nullptr;        // 非所有
  Camera *camera_ = nullptr;                // 非所有（黒板をカメラ前に置く用）
  std::unique_ptr<Object3d> obj_ = nullptr; // 黒フェード板（Object3d）

  // 黒板のα（0..1）
  float alpha_ = 0.0f;

  // 時間
  float timer_ = 0.0f;

  // ===== 調整パラメータ（ここだけ触れば見た目変わる） =====
  // FadeIn: 黒が 1->0 になる時間
  float fadeInTime_ = 1.0f;

  // FadeOut: 泡を先に見せる時間（この間は黒α=0固定）
  float fadeOutDelay_ = 0.35f;

  // FadeOut: 黒が 0->1 になる時間
  float fadeOutTime_ = 0.75f;

  // FadeIn: 黒が0になった後、泡が消えるのを待つ時間（Emit停止して自然消滅待ち）
  float bubbleVanishWait_ = 0.35f;

  // 開始直後の「ドバッ」で謎の間を消す
  uint32_t burstIn_ = 250;
  uint32_t burstOut_ = 200;

  // ブクブク強さ
  uint32_t emitInPerFrame_ = 20;
  uint32_t emitOutPerFrame_ = 25;

  // ==================================================

  bool finished_ = false;
  bool covering_ = false;

  // FadeOutで黒が1になったら泡生成停止
  bool stopEmit_ = false;

private:
  void UpdateOverlayTransform_();
  void SetOverlayAlpha_(float a);
  void EmitBubblesIn_();
  void EmitBubblesOut_(float intensity01);
};
