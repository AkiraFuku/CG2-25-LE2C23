#define NOMINMAX
#include "GameScene.h"

#include "Input.h"
#include "ModelManager.h"
#include "SceneManager.h"

#include "MathFunction.h"

#include "imgui.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr float kPi = 3.14159265358979323846f;
inline float Lerp(float a, float b, float t) { return a + (b - a) * t; }
} // namespace

//========================
// Math helpers (Vector3)
//========================
namespace {
inline Vector3 ClampVec3(const Vector3 &v, const Vector3 &mn,
                         const Vector3 &mx) {
  return Vector3{
      std::clamp(v.x, mn.x, mx.x),
      std::clamp(v.y, mn.y, mx.y),
      std::clamp(v.z, mn.z, mx.z),
  };
}
} // namespace

namespace {
// yaw(y)・pitch(x) から前方向ベクトルを作る（Z+ が前想定）
inline Vector3 ForwardFromYawPitch(float pitch, float yaw) {
  const float cp = std::cos(pitch);
  const float sp = std::sin(pitch);
  const float cy = std::cos(yaw);
  const float sy = std::sin(yaw);

  // forward: (x, y, z)
  return Normalize(Vector3{sy * cp, sp, cy * cp});
}

// yaw だけから右方向（水平ストレイフ）を作る
inline Vector3 RightFromYaw(float yaw) {
  const float cy = std::cos(yaw);
  const float sy = std::sin(yaw);
  // right: (x, y, z)
  return Normalize(Vector3{cy, 0.0f, -sy});
}
} // namespace

//========================
// GameScene
//========================
void GameScene::Initialize() {

  // -----------------
  // Camera
  // -----------------
  camera = std::make_unique<Camera>();
  camera->SetRotate({0.0f, 0.0f, 0.0f});
  camera->SetTranslate({0.0f, 2.5f, -6.0f});
  Object3dCommon::GetInstance()->SetDefaultCamera(camera.get());
  ParticleManager::GetInstance()->Setcamera(camera.get());

  // -----------------
  // Audio / Texture
  // -----------------
  soundData1 = Audio::GetInstance()->SoundLoadWave("resources/fanfare.mp3");
  Audio::GetInstance()->PlayAudio(soundData1);

  TextureManager::GetInstance()->LoadTexture("resources/uvChecker.png");
  ParticleManager::GetInstance()->CreateParticleGroup(
      "Test", "resources/uvChecker.png");

  // -----------------
  // Sprite (debug)
  // -----------------
  sprite = std::make_unique<Sprite>();
  sprite->Initialize("resources/uvChecker.png");
  sprite->SetPosition(Vector2{125.0f, 100.0f});
  sprite->SetAnchorPoint(Vector2{0.5f, 0.5f});

  // -----------------
  // Debug objects
  // -----------------
  object3d2 = std::make_unique<Object3d>();
  object3d2->Initialize();
  object3d = std::make_unique<Object3d>();
  object3d->Initialize();

  // NOTE: 既にあなたの環境にあるモデル名を使っています。
  //       無ければ手持ちの obj 名に変えてOK。
  ModelManager::GetInstance()->LoadModel("plane.obj");
  ModelManager::GetInstance()->LoadModel("axis.obj");
  ModelManager::GetInstance()->LoadModel("cube.obj");

  object3d2->SetTranslate(Vector3{0.0f, 10.0f, 0.0f});
  object3d2->SetModel("axis.obj");
  object3d->SetModel("plane.obj");

  // -----------------
  // CourseWall（壁管理）※ unique_ptr化
  // -----------------
  courseWall_ = std::make_unique<CourseWall>();
  courseWall_->Initialize("cube.obj");

  // -----------------
  // Particle emitter (existing)
  // -----------------
  Transform M = {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
  emitter = std::make_unique<ParicleEmitter>("Test", M, 10, 5.0f, 0.0f);

  // -----------------
  // Car (demo)
  // -----------------
  car_.object = std::make_unique<Object3d>();
  car_.object->Initialize();
  car_.object->SetModel("cube.obj"); // 目印
  car_.pos = Vector3{-13.0f, car_.radius, 0.0f};

  // -----------------
  // Course walls
  // -----------------
  BuildCourseWalls();
}

void GameScene::Finalize() {
  ParticleManager::GetInstance()->ReleaseParticleGroup("Test");

  // ★ unique_ptrなのでこれでOK（必要なら reset 前に Clear を呼んでもよい）
  if (courseWall_) {
    courseWall_->Clear();
  }
  courseWall_.reset();
}

void GameScene::Update() {
  emitter->Update();

  //====================================
  // 入力：axis(=car_) をパッド + キーボードで手動操作
  //====================================
  Vector3 wishMove{0.0f, 0.0f, 0.0f};

  // ---- pad ----
  XINPUT_STATE state{};
  const bool hasPad = Input::GetInstance()->GetJoyStick(0, state);
  if (hasPad) {
    const float x = (float)state.Gamepad.sThumbLX / 32767.0f;
    const float y = (float)state.Gamepad.sThumbLY / 32767.0f;
    wishMove.x += x;
    wishMove.z += y;
  }

  // ---- keyboard ----
  if (Input::GetInstance()->PushedKeyDown(DIK_A))
    wishMove.x -= 1.0f;
  if (Input::GetInstance()->PushedKeyDown(DIK_D))
    wishMove.x += 1.0f;
  if (Input::GetInstance()->PushedKeyDown(DIK_W))
    wishMove.z += 1.0f;
  if (Input::GetInstance()->PushedKeyDown(DIK_S))
    wishMove.z -= 1.0f;

  if (Input::GetInstance()->PushedKeyDown(DIK_SPACE))
    wishMove.y += 1.0f;
  if (Input::GetInstance()->PushedKeyDown(DIK_LCONTROL))
    wishMove.y -= 1.0f;

  //====================================
  // Aボタンでタイトルへ（既存）
  //====================================
  if (Input::GetInstance()->TriggerPadDown(0, XINPUT_GAMEPAD_A)) {
    GetSceneManager()->ChangeScene("TitleScene");
  }

  // マウスホイールでカメラ前後（既存）
  if (Input::GetInstance()->GetMouseMove().z) {
    Vector3 camreaTranslate = camera->GetTranslate();
    camreaTranslate =
        Add(camreaTranslate,
            Vector3{0.0f, 0.0f,
                    static_cast<float>(Input::GetInstance()->GetMouseMove().z) *
                        0.1f});
    camera->SetTranslate(camreaTranslate);
  }

  // 速度
  float speed = carMoveSpeed_;
  if (Input::GetInstance()->PushedKeyDown(DIK_LSHIFT)) {
    speed *= 2.5f;
  }

  // 斜め移動で速くならないよう正規化
  const Vector3 dir = Normalize(wishMove);

  // 60fps想定の簡易
  car_.vel = Multiply(speed / 60.0f, dir);
  car_.pos = Add(car_.pos, car_.vel);

  // 「地面に貼り付け」するなら固定（上下移動を無効化）
  car_.pos.y = car_.radius;

  //====================================
  // 壁との当たり判定（球 vs 壁）
  //====================================
  if (courseWall_) {
    courseWall_->ResolveSphere(car_.pos, car_.vel, car_.radius);
  }

  //====================================
  // マウス左ドラッグでカメラ回転（Yaw/Pitch）
  //====================================
  {
    // 1px動いた時に回る量（好みで調整）
    constexpr float kMouseRotSpeed = 0.0035f;
    // 見上げ/見下げの限界（ラジアン）
    constexpr float kPitchMin = -1.3f;
    constexpr float kPitchMax = 1.3f;

#ifdef USE_IMGUI
    // ImGui操作中はカメラを動かさない
    if (!ImGui::GetIO().WantCaptureMouse)
#endif
    {
      if (Input::GetInstance()->PushMouseDown(Input::MouseButton::Left)) {
        const auto m = Input::GetInstance()->GetMouseMove(); // LONG x,y,z
        Vector3 rot = camera->GetRotate();

        // マウス移動量で回転（左右=Yaw, 上下=Pitch）
        rot.y += static_cast<float>(m.x) * kMouseRotSpeed;
        rot.x += static_cast<float>(m.y) * kMouseRotSpeed;

        // ピッチ制限（首が後ろに回らないように）
        rot.x = std::clamp(rot.x, kPitchMin, kPitchMax);

        camera->SetRotate(rot);
      }
    }
  }

  camera->Update();
  object3d->Update();
  object3d2->Update();

  // Car Object3d
  if (car_.object) {
    car_.object->SetTranslate(car_.pos);
    car_.object->Update();
  }

  // Wall Object3d
  if (courseWall_) {
    courseWall_->Update();
  }

#ifdef USE_IMGUI
  ImGui::Begin("Debug");

  ImGui::Text("Course");
  bool rebuild = false;
  rebuild |= ImGui::SliderFloat("HalfWidth", &courseHalfWidth_, 0.5f, 8.0f);
  rebuild |= ImGui::SliderFloat("WallHeight", &wallHeight_, 0.3f, 3.0f);
  rebuild |= ImGui::SliderFloat("WallThickness", &wallThickness_, 0.05f, 1.0f);
  rebuild |= ImGui::SliderFloat("CarSpeed", &carMoveSpeed_, 1.0f, 20.0f);
  ImGui::Text("CarPos: (%.2f, %.2f, %.2f)", car_.pos.x, car_.pos.y, car_.pos.z);
  if (rebuild) {
    BuildCourseWalls();
  }

  ImGui::End();

  //====================================
  // ImGui: Camera Move (前後左右上下)
  //====================================
  ImGui::Separator();
  ImGui::Text("Camera");

  static bool camMoveEnable = true;
  static float camMoveSpeed = 6.0f; // units/sec
  ImGui::Checkbox("Enable ImGui Camera Move", &camMoveEnable);
  ImGui::SliderFloat("CamMoveSpeed", &camMoveSpeed, 0.1f, 50.0f);

  // 現在値表示＆手入力でもいじれるように
  Vector3 camPos = camera->GetTranslate();
  Vector3 camRot = camera->GetRotate();
  ImGui::DragFloat3("CamPos", &camPos.x, 0.05f);
  ImGui::DragFloat3("CamRot", &camRot.x, 0.01f); // ラジアン想定
  camera->SetTranslate(camPos);
  camera->SetRotate(camRot);

  // ボタン押しっぱなしで動く（IsItemActive）
  if (camMoveEnable) {
    // ImGui操作中だけカメラを動かすので、ここは WantCaptureMouse でもOK
    // 速度を 60fps 想定で 1フレーム分に
    const float step = camMoveSpeed / 60.0f;

    const float pitch = camera->GetRotate().x;
    const float yaw = camera->GetRotate().y;

    const Vector3 forward = ForwardFromYawPitch(pitch, yaw);
    const Vector3 right = RightFromYaw(yaw);
    const Vector3 up = Vector3{0.0f, 1.0f, 0.0f};

    Vector3 pos = camera->GetTranslate();

    ImGui::Text("Hold buttons to move");
    if (ImGui::Button("Forward")) {
    }
    if (ImGui::IsItemActive()) {
      pos = Add(pos, Multiply(step, forward));
    }

    ImGui::SameLine();
    if (ImGui::Button("Back")) {
    }
    if (ImGui::IsItemActive()) {
      pos = Add(pos, Multiply(-step, forward));
    }

    if (ImGui::Button("Right")) {
    }
    if (ImGui::IsItemActive()) {
      pos = Add(pos, Multiply(step, right));
    }

    ImGui::SameLine();
    if (ImGui::Button("Left")) {
    }
    if (ImGui::IsItemActive()) {
      pos = Add(pos, Multiply(-step, right));
    }

    if (ImGui::Button("Up")) {
    }
    if (ImGui::IsItemActive()) {
      pos = Add(pos, Multiply(step, up));
    }

    ImGui::SameLine();
    if (ImGui::Button("Down")) {
    }
    if (ImGui::IsItemActive()) {
      pos = Add(pos, Multiply(-step, up));
    }

    camera->SetTranslate(pos);
  }

#endif

  sprite->Update();
}

void GameScene::Draw() {
  // 3D
  if (courseWall_) {
    courseWall_->Draw();
  }

  if (car_.object) {
    if (!courseWall_->IsCollision()) {
      // SE鳴らす
      // ダメージ
      // エフェクト
      // ImGui表示
    car_.object->Draw();
    }
  }

  ParticleManager::GetInstance()->Draw();

  // 2D
  // sprite->Draw();
}

//==================================================
// 壁生成（0の形）
//==================================================
void GameScene::BuildCourseWalls() {
  if (!courseWall_)
    return;

  std::vector<Vector3> centerLine;
  centerLine.reserve(1024);

  const float y = 0.0f;

  // ========= 0っぽさ調整 =========
  const float radius = 12.0f;      // 上下の丸み（半円半径）
  const float straightLen = 80.0f; // ★ここを長くすると「0」になる（短いとO）
  const float step = 1.0f;         // 直線の点間隔
  const int arcSeg = 48;           // 半円の分割数（滑らかさ）

  // 上端・下端の円弧中心（Z方向に直線がある想定）
  const float zTop = +straightLen * 0.5f;
  const float zBot = -straightLen * 0.5f;

  const Vector3 cTop{0.0f, y, zTop};
  const Vector3 cBot{0.0f, y, zBot};

  // ========= 0型を一周（時計回り） =========
  // 左直線：下→上（X=-radius）
  CourseWall::AddStraight(centerLine, Vector3{-radius, y, zBot},
                          Vector3{-radius, y, zTop}, step);

  // 上半円：左→右（中心 cTop） 角度 pi -> 0
  CourseWall::AddArc(centerLine, cTop, radius, kPi, 0.0f, arcSeg);

  // 右直線：上→下（X=+radius）
  CourseWall::AddStraight(centerLine, Vector3{+radius, y, zTop},
                          Vector3{+radius, y, zBot}, step);

  // 下半円：右→左（中心 cBot）
  CourseWall::AddArc(centerLine, cBot, radius, 2.0f * kPi, kPi, arcSeg);

  // 閉じる（最後→最初）
  if (!centerLine.empty()) {
    centerLine.push_back(centerLine.front());
  }

  courseWall_->BuildFromCenterLine(centerLine, courseHalfWidth_, wallHeight_,
                                   wallThickness_);
}
