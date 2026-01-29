//====================================================
// Fade.cpp
//====================================================
#include "Fade.h"

#include "BubbleParticle.h"
#include "Camera.h"
#include "DXCommon.h"
#include "ModelManager.h"

#include <algorithm>
#include <cmath>

namespace {
// 黒板をカメラの少し前に置く距離
constexpr float kOverlayDist = 1.0f;
// 画面を覆うためデカめ（FOV計算しない簡易版）
constexpr Vector3 kOverlayScale = {100.0f, 100.0f, 100.0f};

// rotate(x=pitch, y=yaw)前提の前方向（yaw=0で+Z前提）
// 環境によって前方向が逆なら、distをマイナスにするか式を調整
static Vector3 ForwardFromRot(const Vector3 &rot) {
  const float pitch = rot.x;
  const float yaw = rot.y;

  const float cp = std::cos(pitch);
  const float sp = std::sin(pitch);
  const float cy = std::cos(yaw);
  const float sy = std::sin(yaw);

  return Vector3{sy * cp, -sp, cy * cp};
}
} // namespace

void Fade::Initialize(Camera *camera) {
  camera_ = camera;

  // ----------------------------
  // 泡（既存）
  // ----------------------------
  bubble_ = BubbleParticle::GetInstance(); // 非所有

  static bool initialized = false;
  if (!initialized) {
    bubble_->Initialize("resources/bubble.png");
    initialized = true;
  }
  bubble_->SetCamera(camera_);

  // ----------------------------
  // 黒フェード板（Object3d）
  // ----------------------------
  obj_ = std::make_unique<Object3d>();
  obj_->Initialize();

  ModelManager::GetInstance()->LoadModel("plane.obj");
  obj_->SetModel("plane.obj");

  // ✅ 黒フェードは加算(Add)じゃ絶対見えないので Normal(αブレンド)
  obj_->SetBlendMode(BlendMode::Normal);
  obj_->SetDepthMode(DepthMode::NoDepth);

  obj_->SetTranslate({0.0f, 0.0f, 1000.0f});
  obj_->SetScale(kOverlayScale);
  obj_->SetCamera(camera_);

  // 初期は透明（※必要なら1.0fにしてもOK）
  alpha_ = 0.0f;
  SetOverlayAlpha_(alpha_);

  // 状態リセット
  phase_ = Phase::kIdle;
  timer_ = 0.0f;
  finished_ = false;
  covering_ = false;
  stopEmit_ = false;
}

void Fade::Start(Phase phase) {
  if (phase == Phase::kIdle)
    return;

  phase_ = phase;
  timer_ = 0.0f;
  finished_ = false;
  covering_ = false;
  stopEmit_ = false;

  switch (phase_) {
  case Phase::kFadeIn: {
    // 黒から開始
    alpha_ = 1.0f;
    SetOverlayAlpha_(alpha_);

    // 泡：即見えるように最初にドバッ＋継続Emit（Updateで）
    bubble_->Clear();
    bubble_->EmitFromBottom(
        /*y=*/0.0f,
        /*xMin=*/-12.0f, /*xMax=*/12.0f,
        /*zMin=*/-2.0f, /*zMax=*/2.0f,
        /*count=*/burstIn_);
    break;
  }

  case Phase::kFadeOut: {
    // 透明から開始
    alpha_ = 0.0f;
    SetOverlayAlpha_(alpha_);

    // 泡：最初からブクブク＋最初にドバッ
    bubble_->Clear();
    bubble_->EmitFromBottom(
        /*y=*/-1.0f,
        /*xMin=*/-12.0f, /*xMax=*/12.0f,
        /*zMin=*/-3.0f, /*zMax=*/3.0f,
        /*count=*/burstOut_);
    break;
  }

  default:
    break;
  }
}

void Fade::UpdateOverlayTransform_() {
  if (!obj_ || !camera_)
    return;

  Vector3 camPos = camera_->GetTranslate();
  Vector3 camRot = camera_->GetRotate();
  Vector3 fwd = ForwardFromRot(camRot);

  Vector3 pos = camPos;
  pos = Add(pos, Vector3{fwd.x * kOverlayDist, fwd.y * kOverlayDist,
                         fwd.z * kOverlayDist});

  obj_->SetTranslate(pos);
  obj_->SetRotate(camRot);

  // Object3dのUpdate（GameScene参考）
  obj_->Update();
}

void Fade::SetOverlayAlpha_(float a) {
  alpha_ = std::clamp(a, 0.0f, 1.0f);
  // あなたが追加した SetColor が効く前提
  obj_->SetColor(Vector4{0.0f, 0.0f, 0.0f, alpha_});
}

void Fade::EmitBubblesIn_() {
  // FadeIn: 黒が薄れる間、泡をブクブク（常時）
  bubble_->EmitFromBottom(
      /*y=*/-0.5f,
      /*xMin=*/-12.0f, /*xMax=*/12.0f,
      /*zMin=*/-2.0f, /*zMax=*/2.0f,
      /*count=*/emitInPerFrame_);
}

void Fade::EmitBubblesOut_(float intensity01) {
  // FadeOut: 強度に応じてEmit量を増やす（それなりに覆う感じに）
  uint32_t c = static_cast<uint32_t>(emitOutPerFrame_ * (0.5f + intensity01));
  bubble_->EmitFromBottom(
      /*y=*/-1.0f,
      /*xMin=*/-12.0f, /*xMax=*/12.0f,
      /*zMin=*/-3.0f, /*zMax=*/3.0f,
      /*count=*/c);
}

void Fade::Update() {
  UpdateOverlayTransform_();

  // ★ dtがデカいと1フレで終わるので上限をつける
  float dt = DXCommon::kDeltaTime;
  dt = std::min(dt, 1.0f / 30.0f); // 33ms上限（好みで 1/60 でも）

  bubble_->Update(
      /*dtが渡せるならdtを渡す*/); // 渡せないなら内部も同様にクランプ推奨

  if (phase_ == Phase::kIdle)
    return;

  timer_ += dt;

  switch (phase_) {
  //==================================================
  // FadeIn
  // 黒(1) + 泡ブクブク開始 → 黒が1->0
  // 黒が0になったら泡の生成を止めて自然消滅待ち → 完了
  //==================================================
  case Phase::kFadeIn: {
    // 黒の進行（1->0）
    float t = std::clamp(timer_ / fadeInTime_, 0.0f, 1.0f);
    SetOverlayAlpha_(1.0f - t);

    // 黒がまだ残ってる間は泡を出し続ける
    if (t < 1.0f) {
      EmitBubblesIn_();
    }

    // 黒が完全に透明になったら、泡の生成を止めて「消える待ち」
    if (t >= 1.0f) {
      // この時点で黒は0固定
      SetOverlayAlpha_(0.0f);

      // 少し待って泡が消えた頃に完了
      if (timer_ >= fadeInTime_ + bubbleVanishWait_) {
        finished_ = true;
        phase_ = Phase::kIdle;
      }
    }
    break;
  }

  //==================================================
  // FadeOut
  // 泡を先にブクブク → delay後に黒0->1開始
  // 黒が1になったら泡生成停止＆covering/finished
  //==================================================
  case Phase::kFadeOut: {

    // 泡：黒が1になるまで出す（stopEmit_フラグで制御）
    if (!stopEmit_) {
      float intensity01 = 0.0f;
      if (timer_ > fadeOutDelay_) {
        float tt = (timer_ - fadeOutDelay_) / fadeOutTime_;
        intensity01 = std::clamp(tt, 0.0f, 1.0f);
      }
      EmitBubblesOut_(intensity01);
    }

    // 黒：delay中は0、その後0->1
    if (timer_ < fadeOutDelay_) {
      SetOverlayAlpha_(0.0f);
    } else {
      float tt =
          std::clamp((timer_ - fadeOutDelay_) / fadeOutTime_, 0.0f, 1.0f);
      SetOverlayAlpha_(tt);

      // 黒板が完全に画面を覆った (alpha >= 1.0)
      if (tt >= 1.0f) {
        stopEmit_ = true; // 新規生成を停止
         // 画面は隠れている状態とする

        // ★ここが重要：泡がリストから完全になくなるまで待機
        if (bubble_->IsEmpty()) {
          finished_ = true;
          covering_ = true;
          phase_ = Phase::kIdle;
        }
      }
    }
    break;
  }

  default:
    break;
  }
}

void Fade::Draw() {
  // 黒で暗くしつつ泡を前面で見せたい → 黒→泡
  if (alpha_ > 0.001f) {
    obj_->Draw();
  }
  bubble_->Draw();
}
