#define NOMINMAX
#include "GameScene.h"
#include "Input.h"
#include "ModelManager.h"
#include "ObstacleSlow.h"
#include "Player.h"
#include "SceneManager.h"
#include "TitleScene.h"
#include "imgui.h"

#include <fstream>
#include <sstream>

#include <cmath>
#include <cctype>

// コンストラクタ
GameScene::GameScene() = default;

// デストラクタ
GameScene::~GameScene() = default;

void GameScene::Initialize() {

  camera = std::make_unique<Camera>();
  camera->SetRotate({0.3f, 0.0f, 0.0f});
  camera->SetTranslate({0.0f, 5.0f, -20.0f});
  Object3dCommon::GetInstance()->SetDefaultCamera(camera.get());
  ParticleManager::GetInstance()->Setcamera(camera.get());

  soundData1 = Audio::GetInstance()->SoundLoadWave("resources/fanfare.mp3");

  Audio::GetInstance()->PlayAudio(soundData1);

  TextureManager::GetInstance()->LoadTexture("resources/uvChecker.png");

  ParticleManager::GetInstance()->CreateParticleGroup(
      "Test", "resources/uvChecker.png");
  /*   std::vector<Sprite*> sprites;
     for (uint32_t i = 0; i < 5; i++)
     {*/
  sprite = std::make_unique<Sprite>();
  // sprite->Initialize(spritecommon,"resources/monsterBall.png");
  sprite->Initialize("resources/uvChecker.png");

  sprite->SetPosition(Vector2{25.0f + 100.0f, 100.0f});
  // sprite->SetSize(Vector2{ 100.0f,100.0f });
  // sprites.push_back(sprite);

  sprite->SetAnchorPoint(Vector2{0.5f, 0.5f});

  //}

  // object3d の初期化
  object3d2 = std::make_unique<Object3d>();
  object3d2->Initialize();

  object3d = std::make_unique<Object3d>();
  object3d->Initialize();

  ModelManager::GetInstance()->LoadModel("plane.obj");
  ModelManager::GetInstance()->LoadModel("axis.obj");
  object3d2->SetTranslate(Vector3{0.0f, 10.0f, 0.0f});
  object3d2->SetModel("axis.obj");
  object3d->SetModel("plane.obj");
  Transform M = {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
  emitter = std::make_unique<ParicleEmitter>("Test", M, 10, 5.0f, 0.0f);

  // プレイヤーの初期化
  player_ = std::make_unique<Player>();
  playerModel_ = std::make_unique<Object3d>();
  playerModel_->SetTranslate(Vector3{0.0f, 0.0f, 0.0f});
  playerModel_->SetModel("cube.obj");
  playerModel_->Initialize();
  player_->Initialize(playerModel_.get(), camera.get(),
                      Vector3{0.0f, 0.0f, 0.0f});

  // 障害物の初期化
  obstacleSlow_ = std::make_unique<ObstacleSlow>();
  obstacleSlowModel_ = std::make_unique<Object3d>();
  obstacleSlowModel_->SetTranslate(Vector3{0.0f, 0.0f, 0.0f});
  obstacleSlowModel_->SetModel("cube.obj");
  obstacleSlowModel_->Initialize();
  obstacleSlow_->Initialize(obstacleSlowModel_.get(), camera.get(),
                            Vector3{0.0f, 0.0f, 40.0f});

  ModelManager::GetInstance()->LoadModel("cube.obj"); // 念のため

LoadCourseFromCommandCsv("resources/course/course.csv", "cube.obj");
}
void GameScene::Finalize() {

  ParticleManager::GetInstance()->ReleaseParticleGroup("Test");
}
void GameScene::Update() {
  emitter->Update();

  XINPUT_STATE state;

  // 現在のジョイスティックを取得

  Input::GetInstance()->GetJoyStick(0, state);

  // Aボタンを押していたら

  if (Input::GetInstance()->TriggerPadDown(0, XINPUT_GAMEPAD_A)) {

    // Aボタンを押したときの処理

    GetSceneManager()->ChangeScene("TitleScene");
  }
  if (Input::GetInstance()->TriggerPadDown(0, XINPUT_GAMEPAD_B)) {
  }

  // マウスホイールの入力取得

  if (Input::GetInstance()->GetMouseMove().z) {
    Vector3 camreaTranslate = camera->GetTranslate();
    camreaTranslate =
        Add(camreaTranslate,
            Vector3{0.0f, 0.0f,
                    static_cast<float>(Input::GetInstance()->GetMouseMove().z) *
                        0.1f});
    camera->SetTranslate(camreaTranslate);
  }
  if (Input::GetInstance()->GetJoyStick(0, state)) {
    // 左スティックの値を取得
    float x = (float)state.Gamepad.sThumbLX;
    float y = (float)state.Gamepad.sThumbLY;

    // 数値が大きいので正規化（-1.0 ～ 1.0）して使うのが一般的
    float normalizedX = x / 32767.0f;
    float normalizedY = y / 32767.0f;
    Vector3 camreaTranslate = camera->GetTranslate();
    camreaTranslate = Add(camreaTranslate, Vector3{normalizedX / 60.0f,
                                                   normalizedY / 60.0f, 0.0f});
    camera->SetTranslate(camreaTranslate);
  }

  camera->Update();
  object3d->Update();
  object3d2->Update();

  // プレイヤーの更新処理
  player_->Update();

  // 障害物の更新処理
  obstacleSlow_->Update();

  for (auto &w : walls_) {
    w.Update();
  }


  // 全ての当たり判定を行う
  CheckAllCollisions();

#ifdef USE_IMGUI
  ImGui::Begin("Debug");

  ImGui::Text("Sprite");
  Vector2 Position = sprite->GetPosition();
  ImGui::SliderFloat2("Position", &(Position.x), 0.1f, 1000.0f);
  sprite->SetPosition(Position);
  ImGui::End();
#endif // USE_IMGUI

  // sprite->SetRotation(sprite->GetRotation() + 0.1f);
  sprite->Update();
}
void GameScene::Draw() {
  // object3d2->Draw();
  // object3d->Draw();
  // ParticleManager::GetInstance()->Draw();

  // プレイヤーの描画処理
  player_->Draw();

  for (auto &w : walls_) {
    w.Draw();
  }

  // 障害物の描画処理
  obstacleSlow_->Draw();

  ///////スプライトの描画
  // sprite->Draw();
}

static Vector3 GetAabbCenter(const AABB &a) {
  return Vector3{
      (a.min.x + a.max.x) * 0.5f,
      (a.min.y + a.max.y) * 0.5f,
      (a.min.z + a.max.z) * 0.5f,
  };
}

static Vector3 GetAabbHalf(const AABB &a) {
  return Vector3{
      (a.max.x - a.min.x) * 0.5f,
      (a.max.y - a.min.y) * 0.5f,
      (a.max.z - a.min.z) * 0.5f,
  };
}

// プレイヤー(pos)を壁の外へ押し出す（XZメイン）
// 戻り値：押し出したなら true
static bool ResolveAabbXZ(Vector3 &inOutPlayerPos, const AABB &playerAabb,
                          const AABB &wallAabb) {
  // center/half
  const Vector3 pc = GetAabbCenter(playerAabb);
  const Vector3 ph = GetAabbHalf(playerAabb);

  const Vector3 wc = GetAabbCenter(wallAabb);
  const Vector3 wh = GetAabbHalf(wallAabb);

  // 差分
  const float dx = pc.x - wc.x;
  const float dz = pc.z - wc.z;

  // 重なり量（+なら重なってる）
  const float overlapX = (ph.x + wh.x) - std::abs(dx);
  const float overlapZ = (ph.z + wh.z) - std::abs(dz);

  if (overlapX <= 0.0f || overlapZ <= 0.0f) {
    return false; // 重なってない
  }

  // 少ない方の軸に押し出す（マリオカートっぽい）
  if (overlapX < overlapZ) {
    // X方向へ
    inOutPlayerPos.x += (dx >= 0.0f) ? overlapX : -overlapX;
  } else {
    // Z方向へ
    inOutPlayerPos.z += (dz >= 0.0f) ? overlapZ : -overlapZ;
  }

  return true;
}

static AABB MakePlayerAabbAt(const Vector3 &pos) {
  // Player::GetAABB と同じサイズを使う必要がある
  // kWidth/kHeight は Player.cpp にあるのでここでは値が見えない
  // → ここだけは Playerの定数に合わせて手で合わせる or
  // Playerに関数を作るのが正解

  constexpr float kWidth = 1.0f;  // ←あなたのPlayerのkWidthに合わせて
  constexpr float kHeight = 1.0f; // ←あなたのPlayerのkHeightに合わせて

  AABB aabb;
  aabb.min = {pos.x - kWidth / 2.0f, pos.y - kHeight / 2.0f,
              pos.z - kWidth / 2.0f};
  aabb.max = {pos.x + kWidth / 2.0f, pos.y + kHeight / 2.0f,
              pos.z + kWidth / 2.0f};
  return aabb;
}

void GameScene::CheckAllCollisions() {
#pragma region 自キャラと障害物(ゆっくり)の当たり判定
  // 判定対象1と2の座標
  AABB aabb1, aabb2;

  // 自キャラの座標
  aabb1 = player_->GetAABB();

  // 障害物(ゆっくり)の座標
  aabb2 = obstacleSlow_->GetAABB();

  // 当たり判定
  if (isCollision(aabb1, aabb2)) {
    // 衝突応答
    obstacleSlow_->OnCollision(player_.get());
  }

#pragma endregion
#pragma region 自キャラと壁の当たり判定（押し戻し）
  AABB playerAabb = player_->GetAABB();
  Vector3 playerPos = player_->GetWorldPosition();

  auto ResolveWithWall = [&](const CourseWall &w) {
    const AABB wallAabb = w.GetAABB();
    if (!isCollision(playerAabb, wallAabb)) {
      return;
    }

    if (ResolveAabbXZ(playerPos, playerAabb, wallAabb)) {
      const Vector3 delta = playerPos - player_->GetWorldPosition();
      playerAabb.min += delta;
      playerAabb.max += delta;
    }
  };

  for (const auto &w : walls_) {
    ResolveWithWall(w);
  }

  player_->SetPosition(playerPos);
#pragma endregion
}

bool GameScene::isCollision(const AABB &aabb1, const AABB &aabb2) {
  if (aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x &&
      aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y &&
      aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z) {
    return true;
  }

  return false;
}

// --------------- 便利 ---------------
struct SegmentCmd {
  Vector3 a;
  Vector3 b;
  float halfWidth;
};

static float YawFromDirXZ(const Vector3 &dir) {
  // Z+前提：yaw=0で+Z
  return std::atan2(dir.x, dir.z);
}

static float LengthXZ(const Vector3 &v) {
  return std::sqrt(v.x * v.x + v.z * v.z);
}

static std::string Trim2(const std::string &s) {
  size_t b = 0;
  while (b < s.size() && std::isspace((unsigned char)s[b]))
    ++b;
  size_t e = s.size();
  while (e > b && std::isspace((unsigned char)s[e - 1]))
    --e;
  return s.substr(b, e - b);
}

static std::string ToUpper(std::string s) {
  for (auto &c : s)
    c = (char)std::toupper((unsigned char)c);
  return s;
}

// yaw（Y回転）から前ベクトル（Z+前提）
static Vector3 ForwardFromYaw(float yaw) {
  return Vector3{std::sin(yaw), 0.0f, std::cos(yaw)};
}

static Vector3 LeftFromForward(const Vector3 &fwd) {
  return Vector3{-fwd.z, 0.0f, fwd.x};
}

static bool NearlyEqualXZ(const Vector3 &a, const Vector3 &b,
                          float eps = 1e-4f) {
  return (std::abs(a.x - b.x) < eps) && (std::abs(a.z - b.z) < eps);
}

// step で from->to を点列生成（from含む, to含む）
// ★重複点は入れない
static void AddSegmentPoints(std::vector<Vector3> &out, const Vector3 &from,
                             const Vector3 &to, float step) {
  Vector3 dir = Subtract(to, from);
  dir.y = 0.0f;

  float len = std::sqrt(dir.x * dir.x + dir.z * dir.z);
  if (len <= 1e-6f) {
    if (out.empty() || !NearlyEqualXZ(out.back(), from)) {
      out.push_back(from);
    }
    return;
  }

  Vector3 n = {dir.x / len, 0.0f, dir.z / len};

  int count = (int)std::floor(len / step);

  // ★最初の点(from)がすでに入ってるなら i=1 から
  int startI = 0;
  if (!out.empty() && NearlyEqualXZ(out.back(), from)) {
    startI = 1;
  }

  for (int i = startI; i <= count; ++i) {
    float t = (float)i * step;
    Vector3 p = Add(from, Vector3{n.x * t, 0.0f, n.z * t});
    if (out.empty() || !NearlyEqualXZ(out.back(), p)) {
      out.push_back(p);
    }
  }

  // 終点を保証（ただし重複は避ける）
  if (out.empty() || !NearlyEqualXZ(out.back(), to)) {
    out.push_back(to);
  }
}

// 円弧点列（curPos と curYaw を更新する）
// ★重複点は入れない
static void AddArcPoints(std::vector<Vector3> &out, Vector3 &inOutPos,
                         float &inOutYaw, float radius, float angleDeg,
                         float step, bool turnRight) {
  const float angleRad = angleDeg * 3.1415926535f / 180.0f;

  // yaw定義：yawが増えると右(+X)へ向く → RIGHTは +、LEFTは -
  const float sign = turnRight ? +1.0f : -1.0f;

  // 現在の forward と left / right
  const Vector3 fwd = ForwardFromYaw(inOutYaw);
  const Vector3 left = LeftFromForward(fwd);
  const Vector3 right = Vector3{-left.x, 0.0f, -left.z};

  // 旋回中心（RIGHTなら右側、LEFTなら左側）
  const Vector3 center =
      turnRight
          ? Add(inOutPos, Vector3{right.x * radius, 0.0f, right.z * radius})
          : Add(inOutPos, Vector3{left.x * radius, 0.0f, left.z * radius});

  // center -> start(現在位置) のベクトル（XZ）
  Vector3 startVec = Subtract(inOutPos, center); // 長さ=radiusのはず

  // 分割数
  const float arcLen = radius * angleRad;
  const int div = std::max(1, (int)std::ceil(arcLen / step));
  const float dTheta = angleRad / (float)div;

  // Y回転（あなたの yaw と同じ回転行列）
  auto RotateY_XZ = [](const Vector3 &v, float theta) -> Vector3 {
    const float c = std::cos(theta);
    const float s = std::sin(theta);
    //  yawと一致：x' = x*c + z*s, z' = -x*s + z*c
    return Vector3{
        v.x * c + v.z * s,
        v.y,
        -v.x * s + v.z * c,
    };
  };

  for (int i = 1; i <= div; ++i) {
    const float theta = sign * dTheta * (float)i;
    Vector3 v = RotateY_XZ(startVec, theta);
    Vector3 p = Add(center, v);
    p.y = inOutPos.y;

    if (!out.empty() && NearlyEqualXZ(out.back(), p)) {
      continue;
    }
    out.push_back(p);
  }

  // 最終更新
  if (!out.empty()) {
    inOutPos = out.back();
  }
  inOutYaw += sign * angleRad;
}


// --------------- メイン：命令CSV読み込み ---------------
// ★この関数を丸ごと差し替え
bool GameScene::LoadCourseFromCommandCsv(const std::string &csvPath,
                                         const std::string &wallModelName) {
  std::ifstream ifs(csvPath);
  if (!ifs.is_open()) {
    return false;
  }

  // 既存クリア
  walls_.clear();
  wallObj_.clear();

  // 壁モデルロード（Modelは共有される）
  ModelManager::GetInstance()->LoadModel(wallModelName);

  std::vector<Vector3> centerLine;
  centerLine.reserve(2048);

  Vector3 curPos{0, 0, 0};
  float curYaw = 0.0f; // Y回転

  float defaultHalfWidth = 8.0f;

  std::string line;
  while (std::getline(ifs, line)) {
    line = Trim2(line);
    if (line.empty())
      continue;
    if (line[0] == '#')
      continue;

    std::stringstream ss(line);
    std::string cmd;
    if (!std::getline(ss, cmd, ','))
      continue;
    cmd = ToUpper(Trim2(cmd));

    // パラメータ読み取り（float）
    std::vector<float> p;
    std::string token;
    while (std::getline(ss, token, ',')) {
      token = Trim2(token);
      if (token.empty())
        continue;
      p.push_back(std::stof(token));
    }

    if (cmd == "START") {
      if (p.size() < 3)
        continue;
      curPos = Vector3{p[0], p[1], p[2]};
      if (centerLine.empty() || !NearlyEqualXZ(centerLine.back(), curPos)) {
        centerLine.push_back(curPos);
      }
    } else if (cmd == "STRAIGHT") {
      // STRAIGHT, length, step, halfWidth(optional)
      if (p.size() < 2)
        continue;
      float length = p[0];
      float step = p[1];
      if (p.size() >= 3)
        defaultHalfWidth = p[2];

      Vector3 fwd = ForwardFromYaw(curYaw);
      Vector3 to = Add(curPos, Vector3{fwd.x * length, 0.0f, fwd.z * length});

      AddSegmentPoints(centerLine, curPos, to, step);
      curPos = to;

    } else if (cmd == "RIGHT" || cmd == "LEFT") {
      // RIGHT/LEFT, radius, angleDeg, step, halfWidth(optional)
      if (p.size() < 3)
        continue;
      float radius = p[0];
      float angleDeg = p[1];
      float step = p[2];
      if (p.size() >= 4)
        defaultHalfWidth = p[3];

      bool turnRight = (cmd == "RIGHT");
      AddArcPoints(centerLine, curPos, curYaw, radius, angleDeg, step,
                   turnRight);
    }
  }

  if (centerLine.size() < 2) {
    return false;
  }

  // 左右壁生成（centerLine の各点で forward を推定して左右にオフセット）
  walls_.reserve(centerLine.size() * 2);
  wallObj_.reserve(centerLine.size() * 2);

  for (size_t i = 0; i < centerLine.size(); ++i) {
    Vector3 p0 = centerLine[i];

    // ★向き推定：基本は次点、最後/重複なら前点も使う
    Vector3 pPrev = (i > 0) ? centerLine[i - 1] : centerLine[i];
    Vector3 pNext =
        (i + 1 < centerLine.size()) ? centerLine[i + 1] : centerLine[i];

    Vector3 dir = Subtract(pNext, p0);
    dir.y = 0.0f;
    float len = std::sqrt(dir.x * dir.x + dir.z * dir.z);

    if (len <= 1e-6f) {
      dir = Subtract(p0, pPrev);
      dir.y = 0.0f;
      len = std::sqrt(dir.x * dir.x + dir.z * dir.z);
    }

    Vector3 fwd = (len > 1e-6f) ? Vector3{dir.x / len, 0.0f, dir.z / len}
                                : Vector3{0, 0, 1};

    Vector3 left = LeftFromForward(fwd);

    Vector3 leftPos = Add(p0, Vector3{left.x * defaultHalfWidth, 0.0f,
                                      left.z * defaultHalfWidth});
    Vector3 rightPos = Add(p0, Vector3{-left.x * defaultHalfWidth, 0.0f,
                                       -left.z * defaultHalfWidth});

    // 左壁
    {
      auto obj = std::make_unique<Object3d>();
      obj->SetModel(wallModelName);
      obj->Initialize();

      CourseWall wall;
      wall.Initialize(obj.get(), camera.get(), leftPos);

      wallObj_.push_back(std::move(obj));
      walls_.push_back(wall);
    }

    // 右壁
    {
      auto obj = std::make_unique<Object3d>();
      obj->SetModel(wallModelName);
      obj->Initialize();

      CourseWall wall;
      wall.Initialize(obj.get(), camera.get(), rightPos);

      wallObj_.push_back(std::move(obj));
      walls_.push_back(wall);
    }
  }

  return true;
}
