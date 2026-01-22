#pragma once
#include <cstdint>
#include <string>

#include "Vector4.h"

// 前方宣言
class Camera;
class BubbleParticle;

/// 泡でフェードするクラス
/// - FadeOut中: 泡を下からぶわぁと生成（画面を覆う演出）
/// - FadeIn中 : 泡の生成は止める（必要なら泡も消す）
class BubbleFade {
public:
  enum class Mode {
    kNone,
    kFadeOut,
    kFadeIn,
  };

  struct Params {
    // フェード時間（秒）
    float fadeOutTime = 1.0f;
    float fadeInTime = 1.0f;

    // 生成レート（個/秒）
    float emitPerSec = 240.0f;

    // 1回Emitの個数（呼び出し回数を減らす用）
    uint32_t emitBatch = 6;

    // 発生範囲（ワールド座標のオフセット幅）
    // basePos を中心に、X/Z にこの範囲で散らす
    float emitRangeX = 8.0f;
    float emitRangeZ = 2.5f;

    // 発生位置の基準（basePosに足す）
    Vector3 emitterBasePos = {0.0f, -2.0f, 0.0f};

    // FadeInに入った瞬間に泡を全消しするか
    bool clearOnFadeInStart = true;
  };

public:
  BubbleFade() = default;
  ~BubbleFade() = default;

  /// camera は借り物（deleteしない）
  /// bubbleTexturePath は泡テクスチャ（uvCheckerでも動く）
  void Initialize(Camera *camera, const std::string &bubbleTexturePath,
                  const Params &params = Params{});

  void Finalize();

  // フェード開始
  void StartFadeOut();
  void StartFadeIn();

  void Update();
  void Draw();

  // 状態問い合わせ
  Mode GetMode() const { return mode_; }
  bool IsActive() const { return mode_ != Mode::kNone; }

  // FadeOut が終わって「覆い完了」したか（ここでScene切替する想定）
  bool IsCoverComplete() const { return coverComplete_; }

  // FadeIn が終わって完全終了したか
  bool IsFinished() const { return finished_; }

  // 調整用
  void SetParams(const Params &p) { params_ = p; }
  const Params &GetParams() const { return params_; }

private:
  void UpdateFadeOut(float dt);
  void UpdateFadeIn(float dt);

private:
  Camera *camera_ = nullptr;

  Params params_{};

  Mode mode_ = Mode::kNone;
  float timer_ = 0.0f;

  // 泡生成の積算
  float emitAccum_ = 0.0f;

  bool coverComplete_ = false;
  bool finished_ = false;

  // BubbleParticle管理
  std::string groupName_ = "FadeBubble";
  std::string texturePath_;
  BubbleParticle *bubble_ = nullptr;
  bool bubbleReady_ = false;
};
