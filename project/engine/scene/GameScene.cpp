#include "GameScene.h"
#include "CourseWall.h"
#include "Input.h"
#include "MapChipField.h"
#include "ModelManager.h"
#include "ObstacleFast.h"
#include "ObstacleMax.h"
#include "ObstacleNormal.h"
#include "ObstacleSlow.h"
#include "PSOMnager.h"
#include "Player.h"
#include "SceneManager.h"
#include "TitleScene.h"
#include "imgui.h"
#include"BubbleFade.h"

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

  handle_ = Audio::GetInstance()->LoadAudio("resources/fanfare.mp3");

  Audio::GetInstance()->PlayAudio(handle_, true);

  TextureManager::GetInstance()->LoadTexture("resources/uvChecker.png");

  ParticleManager::GetInstance()->CreateParticleGroup(
      "Test", "resources/uvChecker.png");
  /*   std::vector<Sprite*> sprites;
     for (uint32_t i = 0; i < 5; i++)
     {*/
  sprite = std::make_unique<Sprite>();

  sprite->Initialize("resources/uvChecker.png");

  sprite->SetPosition(Vector2{25.0f + 100.0f, 100.0f});
  // sprite->SetSize(Vector2{ 100.0f,100.0f });
  // sprites.push_back(sprite);

  sprite->SetBlendMode(BlendMode::Add);
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
  object3d->SetBlendMode(BlendMode::Add);
  Transform M = {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
  emitter = std::make_unique<ParicleEmitter>("Test", M, 10, 5.0f, 0.0f);

  // マップチップフィールドの初期化
  mapChipField_ = std::make_unique<MapChipField>();
  mapChipField_->LoadMapChipCsv("resources/mapchip.csv");

  // マップチップの生成
  GenerateFieldObjects();

  // マップチップデータのセット
  player_->SetMapChipField(mapChipField_.get());

  // Fade作成（FadeがBubbleParticleを生成して管理する）
  fade_ = std::make_unique<BubbleFade>();

  BubbleFade::Params p;
  p.fadeOutTime = 1.0f;
  p.fadeInTime = 0.8f;
  p.emitPerSec = 380.0f; // 量
  p.emitBatch = 10;      // 呼び回数を減らす
  p.emitterBasePos = {0.0f, -2.0f, 0.0f};
  p.clearOnFadeInStart = true; // FadeIn開始で泡リセットするならtrue

  fade_->Initialize(camera.get(), "resources/uvChecker.png", p);
  fade_->StartFadeIn();
}

void GameScene::Finalize() {

  ParticleManager::GetInstance()->ReleaseParticleGroup("Test");

    if (fade_) {
    fade_->Finalize();
  }

  ParticleManager::GetInstance()->ReleaseParticleGroup("Test");
  ParticleManager::GetInstance()->Finalize();
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

  //// マウスホイールの入力取得

  // if (Input::GetInstance()->GetMouseMove().z) {
  //   Vector3 camreaTranslate = camera->GetTranslate();
  //   camreaTranslate =
  //       Add(camreaTranslate,
  //           Vector3{0.0f, 0.0f,
  //                   static_cast<float>(Input::GetInstance()->GetMouseMove().z)
  //                   *
  //                       0.1f});
  //   camera->SetTranslate(camreaTranslate);
  // }

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
  for (auto &obstacle : obstacleSlow_) {
    obstacle->Update();
  }

  for (auto &obstacle : obstacleNormal_) {
    obstacle->Update();
  }

  for (auto &obstacle : obstacleFast_) {
    obstacle->Update();
  }

  for (auto &obstacle : obstacleMax_) {
    obstacle->Update();
  }

  for (auto &wall : walls_) {
    wall->Update();
  }

  // 全ての当たり判定を行う
  CheckAllCollisions();

#ifdef USE_IMGUI
  ImGui::Begin("Debug");

  ImGui::Text("Sprite");
  Vector2 Position = sprite->GetPosition();
  ImGui::SliderFloat2("Position", &(Position.x), 0.1f, 1000.0f);
  sprite->SetPosition(Position);

  ImGui::Text("PlayerSpeed");
  float playerSpeedZ = player_->GetSpeedZ();
  ImGui::SliderFloat("SpeedZ", &playerSpeedZ, 0.0f, 2.0f);

  ImGui::Text("Speed Stage");
  SpeedStage speedStage = player_->GetSpeedStage();
  ImGui::Text("Current Speed Stage: %d", static_cast<int>(speedStage));

  ImGui::End();
#endif // USE_IMGUI

  // sprite->SetRotation(sprite->GetRotation() + 0.1f);
  sprite->Update();

    fade_->Update();
}
void GameScene::Draw() {
  // object3d2->Draw();
  // object3d->Draw();
  // ParticleManager::GetInstance()->Draw();

  // プレイヤーの描画処理
  if (!isDead) {
    player_->Draw();
  }

  // 障害物の描画処理
  for (auto &obstacle : obstacleSlow_) {
    obstacle->Draw();
  }

  for (auto &obstacle : obstacleNormal_) {
    obstacle->Draw();
  }

  for (auto &obstacle : obstacleFast_) {
    obstacle->Draw();
  }

  for (auto &obstacle : obstacleMax_) {
    obstacle->Draw();
  }

  for (auto &wall : walls_) {
    wall->Draw();
  }

  ///////スプライトの描画
  // sprite->Draw();

  fade_->Draw();
}

void GameScene::CheckAllCollisions() {
#pragma region 自キャラと障害物(遅い)の当たり判定
  // 判定対象1と2の座標
  AABB aabbPlayer, aabbSlow;

  // 自キャラの座標
  aabbPlayer = player_->GetAABB();

  // 障害物(遅い)の座標
  for (auto &obstacle : obstacleSlow_) {
    aabbSlow = obstacle->GetAABB();

    // 当たり判定
    if (isCollision(aabbPlayer, aabbSlow)) {
      // 衝突応答
      player_->OnCollision(obstacle.get());
      obstacle->OnCollision(player_.get());
    }
  }
#pragma endregion

#pragma region 自キャラと障害物(普通)の当たり判定
  // 判定対象2の座標
  AABB aabbNormal;

  // 障害物(普通)の座標
  for (auto &obstacle : obstacleNormal_) {
    aabbNormal = obstacle->GetAABB();

    // 当たり判定
    if (isCollision(aabbPlayer, aabbNormal)) {
      // 衝突応答
      player_->OnCollision(obstacle.get());
      obstacle->OnCollision(player_.get());
    }
  }

#pragma endregion

#pragma region 自キャラと障害物(速い)の当たり判定
  // 判定対象2の座標
  AABB aabbFast;

  // 障害物(普通)の座標
  for (auto &obstacle : obstacleFast_) {
    aabbFast = obstacle->GetAABB();

    // 当たり判定
    if (isCollision(aabbPlayer, aabbFast)) {
      // 衝突応答
      player_->OnCollision(obstacle.get());
      obstacle->OnCollision(player_.get());
    }
  }

#pragma endregion

#pragma region 自キャラと障害物(最大速度)の当たり判定
  // 判定対象2の座標
  AABB aabbMax;

  // 障害物(普通)の座標
  for (auto &obstacle : obstacleMax_) {
    aabbMax = obstacle->GetAABB();

    // 当たり判定
    if (isCollision(aabbPlayer, aabbMax)) {
      // 衝突応答
      player_->OnCollision(obstacle.get());
      obstacle->OnCollision(player_.get());
    }
  }

#pragma endregion

#pragma region 自キャラとコースの壁の当たり判定
  AABB aabbWall;
  for (auto &wall : walls_) {
    aabbWall = wall->GetAABB();
    if (isCollision(aabbPlayer, aabbWall)) {
      isDead = true;
      // いったん通知だけでもOK
      // player_->OnCollision(wall.get());  // Player側が対応してるなら
    }
  }
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

void GameScene::GenerateFieldObjects() {
  // 要素数
  uint32_t numBlockVirtical = mapChipField_->GetNumBlockVirtical();
  uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

  // 要素数を変更する
  // 列数の設定
  worldTransformObjects.resize(numBlockVirtical);

  for (uint32_t i = 0; i < numBlockVirtical; ++i) {
    // 一列の要素数を設定(横方向のブロック数)
    worldTransformObjects[i].resize(numBlockHorizontal);
  }

  // オブジェクトの生成
  for (uint32_t i = 0; i < numBlockVirtical; ++i) {
    for (uint32_t j = 0; j < numBlockHorizontal; j++) {

      // 現在の座標のチップタイプを取得
      MapChipType type = mapChipField_->GetMapChipTypeByIndex(j, i);
      Vector3 pos = mapChipField_->GetMapChipPositionByIndex(j, i);

      switch (type) {
      case MapChipType::kBlank:
        break;

      case MapChipType::kPlayer: {
        assert(player_ == nullptr && "自キャラを二重に配置しようとしています");
        // プレイヤーの初期化
        player_ = std::make_unique<Player>();
        playerModel_ = std::make_unique<Object3d>();
        playerModel_->SetTranslate(pos);
        playerModel_->SetModel("cube.obj");
        playerModel_->Initialize();
        player_->Initialize(playerModel_.get(), camera.get(), pos);

      } break;

      case MapChipType::kObstacle: {
        uint8_t subID = mapChipField_->GetMapChipSubIDByIndex(j, i);
        switch (subID) {
        case 0: {
          // モデルを生成してリストに追加
          auto model = std::make_unique<Object3d>();
          model->SetModel("cube.obj");
          model->Initialize();

          // 本体を生成してリストに追加
          auto obstacle = std::make_unique<ObstacleSlow>();
          obstacle->Initialize(model.get(), camera.get(), pos);

          // vectorに保存
          obstacleSlowModel_.push_back(std::move(model));
          obstacleSlow_.push_back(std::move(obstacle));
        } break;

        case 1: {
          // 障害物(普通)の初期化
          // モデルを生成してリストに追加
          auto model = std::make_unique<Object3d>();
          model->SetModel("cube.obj");
          model->Initialize();

          // 本体を生成してリストに追加
          auto obstacle = std::make_unique<ObstacleNormal>();
          obstacle->Initialize(model.get(), camera.get(), pos);

          // vectorに保存
          obstacleNormalModel_.push_back(std::move(model));
          obstacleNormal_.push_back(std::move(obstacle));
        }

        break;

        case 2: {
          // 障害物(速い)の初期化
          // モデルを生成してリストに追加
          auto model = std::make_unique<Object3d>();
          model->SetModel("cube.obj");
          model->Initialize();

          // 本体を生成してリストに追加
          auto obstacle = std::make_unique<ObstacleFast>();
          obstacle->Initialize(model.get(), camera.get(), pos);

          // vectorに保存
          obstacleFastModel_.push_back(std::move(model));
          obstacleFast_.push_back(std::move(obstacle));
        }

        break;

        case 3: {
          // 障害物(最大速度)の初期化
          // モデルを生成してリストに追加
          auto model = std::make_unique<Object3d>();
          model->SetModel("cube.obj");
          model->Initialize();

          // 本体を生成してリストに追加
          auto obstacle = std::make_unique<ObstacleMax>();
          obstacle->Initialize(model.get(), camera.get(), pos);

          // vectorに保存
          obstacleSlowModel_.push_back(std::move(model));
          obstacleMax_.push_back(std::move(obstacle));
        }

        break;
        }
        break;
      }
      case MapChipType::kWallStraight: {
        // 1) モデルを生成してリストに追加
        auto model = std::make_unique<Object3d>();
        model->SetModel("cube.obj"); // ここを壁モデルに（例: "wall.obj" とか）
        model->Initialize();

        // 2) 置く位置を少し上げたいなら（壁が地面に埋まるなら）
        Vector3 wallPos = pos;
        // wallPos.y += 1.0f; // 必要なら

        // 3) 本体を生成してリストに追加
        auto wall = std::make_unique<CourseWall>();
        wall->Initialize(model.get(), camera.get(), wallPos);

        // 4) サイズ調整（マスに合わせる）
        wall->SetScale(Vector3{1.0f, 2.0f, 1.0f}); // 好きに調整

        // 5) subID があるなら向きに使える（例）
        // uint8_t subID = mapChipField_->GetMapChipSubIDByIndex(j, i);
        // if (subID == 1) wall->SetYaw(1.570796f); // 90度

        // 6) vectorに保存
        wallModels_.push_back(std::move(model));
        walls_.push_back(std::move(wall));
      } break;
      case MapChipType::kWallTurn: {
        auto model = std::make_unique<Object3d>();
        model->SetModel("cube.obj");
        model->Initialize();

        Vector3 wallPos = pos; // まず2.0グリッド中心

        const float dx =
            (MapChipField::kBlockWidth - MapChipField::kTurnWidth) *
            0.5f; // 0.5
        const float dz =
            (MapChipField::kBlockHeight - MapChipField::kTurnHeight) *
            0.5f; // 0.5

        uint8_t subID = mapChipField_->GetMapChipSubIDByIndex(j, i);

        switch (subID) {
        case 0:
          wallPos.x += dx;
          wallPos.z += dz;
          break; // 右奥
        case 1:
          wallPos.x -= dx;
          wallPos.z += dz;
          break; // 左奥
        case 2:
          wallPos.x -= dx;
          wallPos.z -= dz;
          break; // 左手前
        case 3:
          wallPos.x += dx;
          wallPos.z -= dz;
          break; // 右手前
        default:
          break;
        }

        auto wall = std::make_unique<CourseWall>();
        wall->Initialize(model.get(), camera.get(), wallPos);

        wall->SetScale(Vector3{1.0f, 2.0f, 1.0f}); // 見た目は小さく

        wallModels_.push_back(std::move(model));
        walls_.push_back(std::move(wall));
      } break;
      }
    }
  }
}
