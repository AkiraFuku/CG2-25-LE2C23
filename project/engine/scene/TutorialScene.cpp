#include "TutorialScene.h"
#include "CourseWall.h"
#include "Input.h"

#include "Goal.h"
#include "LightManager.h"
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
#include <numbers>

// コンストラクタ
TutorialScene::TutorialScene() = default;

// デストラクタ
TutorialScene::~TutorialScene() {
  // メモリ解放
  player_.reset();
  playerModel_.reset();
  obstacleFastModel_.clear();
  obstacleFast_.clear();
  obstacleMaxModel_.clear();
  obstacleMax_.clear();
  obstacleNormalModel_.clear();
  obstacleNormal_.clear();
  obstacleSlowModel_.clear();
  obstacleSlow_.clear();
  wallModels_.clear();
  walls_.clear();
}

void TutorialScene::Initialize() {

  camera = std::make_unique<Camera>();
  camera->SetRotate({0.3f, 0.0f, 0.0f});

  camera->SetTranslate({0.0f, 0.0f, -5.0f});

  Object3dCommon::GetInstance()->SetDefaultCamera(camera.get());
  ParticleManager::GetInstance()->Setcamera(camera.get());

  handle_ = Audio::GetInstance()->LoadAudio("resources/fanfare.mp3");

  Audio::GetInstance()->PlayAudio(handle_, true);

  TextureManager::GetInstance()->LoadTexture("resources/uvChecker.png");

  LightManager::GetInstance()->AddDirectionalLight({1, 1, 1, 1}, {0, -1, 0},
                                                   1.0f); // メインライト

  ParticleManager::GetInstance()->CreateParticleGroup(
      "Test", "resources/uvChecker.png");
  /* std::vector<Sprite*> sprites;
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
  ModelManager::GetInstance()->CreateSphereModel("MySphere", 16);
  object3d->SetBlendMode(BlendMode::Add);
  Transform M = {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
  emitter = std::make_unique<ParicleEmitter>("Test", M, 10, 5.0f, 0.0f);

  // マップチップフィールドの初期化
  mapChipField_ = std::make_unique<MapChipField>();
  mapChipField_->LoadMapChipCsv("resources/stage2.csv");

  // マップチップの生成
  GenerateFieldObjects();

  // マップチップデータのセット
  player_->SetMapChipField(mapChipField_.get());

  // チュートリアル
  currentPhase_ = TutorialPhase::kMovement;
  destroyedObstaclesCount_ = 0;
  isPhaseCleared_ = false;
  waitTimer_ = 0;

  fade_ = std::make_unique<Fade>();
  fade_->Initialize(camera.get());
  sceneState_ = SceneState::kFadeIn;
  fade_->Start(Fade::Phase::kFadeIn);
}

void TutorialScene::Finalize() {

  LightManager::GetInstance()->ClearLights();

  ParticleManager::GetInstance()->ReleaseParticleGroup("Test");
}
void TutorialScene::Update() {

  // 常にフェードの更新は行う
  if (fade_) {
    fade_->Update();
  }

  camera->Update();
  object3d->Update();
  object3d2->Update();

  // プレイヤーの更新処理
  player_->Update(isStarted_);

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

  // ゴールの更新処理
  for (auto &goal : goals_) {
    goal->Update();
  }

  // シーンの状態によって処理を分ける
  switch (sceneState_) {

  //---------------------------------------------------------
  // 1. フェードイン中
  //---------------------------------------------------------
  case SceneState::kFadeIn:
    // フェードインが終わったらメイン状態へ
    if (fade_->IsFinished()) {
      sceneState_ = SceneState::kMain;
    }
    // ※ここでは入力を受け付けないため、returnしたり、下の処理をelseで囲ったりする
    break;

  //---------------------------------------------------------
  // 2. メイン（プレイ可能状態）
  //---------------------------------------------------------
  case SceneState::kMain: {

    if (!isStarted_) {
      isStarted_ = true;
    }

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

    if (Input::GetInstance()->GetJoyStick(0, state)) {
      // 左スティックの値を取得
      float x = (float)state.Gamepad.sThumbLX;
      float y = (float)state.Gamepad.sThumbLY;

      // 数値が大きいので正規化（-1.0 ～ 1.0）して使うのが一般的
      float normalizedX = x / 32767.0f;
      float normalizedY = y / 32767.0f;
      Vector3 camreaTranslate = camera->GetTranslate();
      camreaTranslate =
          Add(camreaTranslate,
              Vector3{normalizedX / 60.0f, normalizedY / 60.0f, 0.0f});
      camera->SetTranslate(camreaTranslate);
    }

    if (player_->IsDead()) {
      if (player_->IsDeathAnimationFinished()) {
        ResetTutorialState();
      }
      // 死んでいる間は操作などを行わないよう break
      break;
    }

    // 当たり判定
    CheckAllCollisions();

    // チュートリアルの更新
    UpdateTutorialSteps();

    auto CheckAndRemove = [&](auto &container) {
      size_t prevSize = container.size();
      container.erase(
          std::remove_if(container.begin(), container.end(),
                         [](const auto &obj) { return obj->IsScoreNone(); }),
          container.end());

      // 減った分だけカウントアップ (攻撃フェーズのみ)
      if (currentPhase_ == TutorialPhase::kAttack) {
        destroyedObstaclesCount_ +=
            static_cast<int>(prevSize - container.size());
      }
    };

    CheckAndRemove(obstacleSlow_);
    CheckAndRemove(obstacleNormal_);
    CheckAndRemove(obstacleFast_);
    CheckAndRemove(obstacleMax_);

    break;
  }

  //---------------------------------------------------------
  // 3. フェードアウト中
  //---------------------------------------------------------
  case SceneState::kFadeOut:
    // フェードアウトが完了したら、次のアクションを実行
    if (fade_->IsFinished()) {
      switch (currentOption_) {
      case CompleteOption::kRetry:
        // リトライ処理
        ResetTutorialState();               // 状態リセット
        currentPhase_ = TutorialPhase::kMovement;
        sceneState_ = SceneState::kFadeIn;  // 状態をフェードインへ
        fade_->SetCamera(camera.get());
        fade_->Start(Fade::Phase::kFadeIn); // フェードイン開始
        break;

      case CompleteOption::kGoToGame:
        GetSceneManager()->ChangeScene("GameScene");
        break;

      case CompleteOption::kBackToTitle:
        GetSceneManager()->ChangeScene("TitleScene");
        break;
      }
    }
    break;
  }

#ifdef USE_IMGUI
  ImGui::Begin("Tutorial");

  const char *phaseStr = "";
  if (isPhaseCleared_) {
    phaseStr = "--- CLEARED! ---";
  } else {
    switch (currentPhase_) {
    case TutorialPhase::kMovement:
      phaseStr = "Step 1: WASD to Move";
      break;
    case TutorialPhase::kDrift:
      phaseStr = "Step 2: SPACE to Brake";
      break;
    case TutorialPhase::kAttack:
      phaseStr = "Step 3: Destroy 3 Obstacles";
      break;

    // ★変更: 完了画面での表示
    case TutorialPhase::kComplete:
      phaseStr = "Tutorial Complete!";

      ImGui::Text("Select Option (W/S or UP/DOWN):");
      ImGui::Separator();

      // Retry
      if (currentOption_ == CompleteOption::kRetry) {
        ImGui::TextColored(ImVec4(1, 1, 0, 1),
                           "> Retry Tutorial"); // 黄色で強調
      } else {
        ImGui::Text("  Retry Tutorial");
      }

      // Go to Game
      if (currentOption_ == CompleteOption::kGoToGame) {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "> Start Game");
      } else {
        ImGui::Text("  Start Game");
      }

      // Back to Title
      if (currentOption_ == CompleteOption::kBackToTitle) {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "> Back to Title");
      } else {
        ImGui::Text("  Back to Title");
      }

      ImGui::Separator();
      ImGui::Text("Press SPACE / A to Confirm");
      break;
    }
  }

  // 完了画面以外では今まで通りのフェーズテキストを表示
  if (currentPhase_ != TutorialPhase::kComplete) {
    ImGui::Text("Tutorial Phase: %s", phaseStr);
  }

  ImGui::End();
// ...
#endif

#ifdef USE_IMGUI
  ImGui::Begin("Debug");
  ImGui::Text("Sphere");
  Vector3 pos = object3d->GetTranslate();
  Vector3 scale = object3d->GetScale();
  ImGui::SliderFloat3("Pos", &(pos.x), 0.1f, 1000.0f);
  ImGui::DragFloat3("scale", &(scale.x), 0.1f, 1000.0f);
  object3d->SetTranslate(pos);
  object3d->SetScale(scale);
  if (LightManager::GetInstance()->GetPointLightCount() > 0) {
    ImGui::Begin("Light Setting");

    // 0番目のポイントライトのデータを参照で取得
    // "auto&"
    // にすることで、ここで書き換えた内容が直接LightManager内のデータに反映されます
    auto &pointLight2 = LightManager::GetInstance()->GetPointLight(1);

    // 位置の調整
    ImGui::DragFloat3("Point Light2 Pos", &pointLight2.position.x, 0.1f);

    // 色の調整
    ImGui::ColorEdit4("Point Light2 Color", &pointLight2.color.x);

    // 強度の調整
    ImGui::DragFloat("Point Light2 Intensity", &pointLight2.intensity, 0.1f,
                     0.0f, 100.0f);

    // 減衰率の調整
    ImGui::DragFloat("Point Light2 Decay", &pointLight2.decay, 0.1f, 0.0f,
                     10.0f);
    auto &pointLight1 = LightManager::GetInstance()->GetPointLight(0);

    // 位置の調整
    ImGui::DragFloat3("Point Light Pos", &pointLight1.position.x, 0.1f);

    // 色の調整
    ImGui::ColorEdit4("Point Light Color", &pointLight1.color.x);

    // 強度の調整
    ImGui::DragFloat("Point Light Intensity", &pointLight1.intensity, 0.1f,
                     0.0f, 100.0f);

    // 減衰率の調整
    ImGui::DragFloat("Point Light Decay", &pointLight1.decay, 0.1f, 0.0f,
                     10.0f);
    ImGui::DragFloat("Point Light rad", &pointLight1.radius, 0.1f, 0.0f, 10.0f);

    ImGui::End();
  }

  // 障害物の削除処理
  obstacleSlow_.erase(
      std::remove_if(obstacleSlow_.begin(), obstacleSlow_.end(),
                     [](const std::unique_ptr<ObstacleSlow> &obstacle) {
                       return obstacle->IsScoreNone();
                     }),
      obstacleSlow_.end());

  obstacleNormal_.erase(
      std::remove_if(obstacleNormal_.begin(), obstacleNormal_.end(),
                     [](const std::unique_ptr<ObstacleNormal> &obstacle) {
                       return obstacle->IsScoreNone();
                     }),

      obstacleNormal_.end());

  obstacleFast_.erase(
      std::remove_if(obstacleFast_.begin(), obstacleFast_.end(),
                     [](const std::unique_ptr<ObstacleFast> &obstacle) {
                       return obstacle->IsScoreNone();
                     }),

      obstacleFast_.end());

  obstacleMax_.erase(
      std::remove_if(obstacleMax_.begin(), obstacleMax_.end(),
                     [](const std::unique_ptr<ObstacleMax> &obstacle) {
                       return obstacle->IsScoreNone();
                     }),

      obstacleMax_.end());

  /*if (!isStarted_) {
    if (countdownTimer_ <= 0) {
      isStarted_ = true;
    } else {
      countdownTimer_--;
    }
  }*/

#ifdef USE_IMGUI
  ImGui::Begin("Debug");

  /* Vector3 direction= object3d->GetDirectionalLightDirection();
   if(ImGui::DragFloat3("Light Direction", &direction.x)){
   object3d->SetDirectionalLightDirection(direction);
   }
   float intensity= object3d->GetDirectionalLightIntensity();
   if(ImGui::InputFloat("intensity",&intensity)){
    object3d->SetDirectionalLightIntensity(intensity);
   }*/

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

  ImGui::Text("Score");
  ImGui::Text("Current Score: %d", player_->GetScore());

  ImGui::End();
#endif // USE_IMGUI

  // sprite->SetRotation(sprite->GetRotation() + 0.1f);
  sprite->Update();

  ImGui::End();
#endif // USE_IMGUI

  // sprite->SetRotation(sprite->GetRotation() + 0.1f);
  sprite->Update();
}

void TutorialScene::Draw() {
  object3d2->Draw();
  object3d->Draw();
  ParticleManager::GetInstance()->Draw();

  // プレイヤーの描画処理

  player_->Draw();

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

  for (auto &goal : goals_) {

    goal->Draw();
  }

  fade_->Draw();

  ///////スプライトの描画
  // sprite->Draw();
}

void TutorialScene::CheckAllCollisions() {

  if (!isStarted_) {
    return;
  }

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

      player_->OnCollision(wall.get());
    }
  }
#pragma endregion

#pragma region 自キャラとゴールの当たり判定
  AABB aabbGoal;
  for (auto &goal : goals_) {
    aabbGoal = goal->GetAABB();
    if (isCollision(aabbPlayer, aabbGoal)) {
      player_->OnCollision(goal.get());
    }
  }

#pragma endregion
}

bool TutorialScene::isCollision(const AABB &aabb1, const AABB &aabb2) {
  if (aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x &&
      aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y &&
      aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z) {
    return true;
  }

  return false;
}

void TutorialScene::GenerateFieldObjects() {
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
      }
    }
  }

  for (uint32_t i = 0; i < numBlockVirtical; ++i) {
    for (uint32_t j = 0; j < numBlockHorizontal; j++) {

      // 現在の座標のチップタイプを取得
      MapChipType type = mapChipField_->GetMapChipTypeByIndex(j, i);
      Vector3 pos = mapChipField_->GetMapChipPositionByIndex(j, i);

      switch (type) {
      case MapChipType::kBlank:
        break;

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
          obstacle->Initialize(model.get(), camera.get(), pos, player_.get());

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
          obstacle->Initialize(model.get(), camera.get(), pos, player_.get());

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
          obstacle->Initialize(model.get(), camera.get(), pos, player_.get());

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
          obstacle->Initialize(model.get(), camera.get(), pos, player_.get());

          // vectorに保存
          obstacleSlowModel_.push_back(std::move(model));
          obstacleMax_.push_back(std::move(obstacle));
        }

        break;
        }
        break;
      }

      case MapChipType::kGoal: {
        auto goalModel = std::make_unique<Object3d>();
        goalModel->SetModel("cube.obj");
        goalModel->Initialize();
        auto goal = std::make_unique<Goal>();
        goal->Initialize(goalModel.get(), camera.get(), pos);
        goalModels_.push_back(std::move(goalModel));
        goals_.push_back(std::move(goal));
      } break;

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
        wall->SetScale(Vector3{2.0f, 4.0f, 2.0f}); // 好きに調整

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
            2.0f; // 0.5
        const float dz =
            (MapChipField::kBlockHeight - MapChipField::kTurnHeight) *
            2.0f; // 0.5

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

        wall->SetScale(Vector3{2.0f, 4.0f, 2.0f}); // 見た目は小さく

        wallModels_.push_back(std::move(model));
        walls_.push_back(std::move(wall));
      } break;
      }
    }
  }
}

void TutorialScene::UpdateTutorialSteps() {
  // ■ 1. すでにクリア条件を満たして待機中の場合
  if (isPhaseCleared_) {
    waitTimer_--;

    // 待機時間が終わったら、次のフェーズへ移行する
    if (waitTimer_ <= 0) {
      isPhaseCleared_ = false; // フラグを下ろす

      // 現在のフェーズに応じて次のフェーズへ遷移
      switch (currentPhase_) {
      case TutorialPhase::kMovement:
        currentPhase_ = TutorialPhase::kDrift;
        break;

      case TutorialPhase::kDrift:
        currentPhase_ = TutorialPhase::kAttack;
        destroyedObstaclesCount_ = 0;
        break;

      case TutorialPhase::kAttack:
        currentPhase_ = TutorialPhase::kComplete;
        break;

      default:
        break;
      }
    }
    // 待機中は、下の判定処理を行わずにここで抜ける
    return;
  }

  // ■ 2. 通常の判定処理 (クリア条件のチェック)
  switch (currentPhase_) {
  case TutorialPhase::kMovement:
    if (Input::GetInstance()->PushedKeyDown(DIK_W) ||
        Input::GetInstance()->PushedKeyDown(DIK_A) ||
        Input::GetInstance()->PushedKeyDown(DIK_S) ||
        Input::GetInstance()->PushedKeyDown(DIK_D)) {

      // 即座にフェーズを変えず、クリアフラグを立ててタイマーセット
      isPhaseCleared_ = true;
      waitTimer_ = kPhaseWaitTime_;
      printf("Movement Cleared! Wait for next...\n");
    }
    break;

  case TutorialPhase::kDrift:
    // Playerクラスの実装では、SPACEを押して旋回し、離した瞬間に加速します。
    // そのため、「現在の速度が目標速度を超えたら」クリアとします。
    // (通常移動だけでは到達しにくい速度を設定しておくと確実です)
    {
      float currentSpeed = player_->GetSpeedZ();
      if (currentSpeed >= kTargetDriftSpeed_) {
        // クリアフラグを立てる
        isPhaseCleared_ = true;
        waitTimer_ = kPhaseWaitTime_;
        printf("Drift Boost Cleared! Wait for next...\n");
      }
    }
    break;

  case TutorialPhase::kAttack:
    if (destroyedObstaclesCount_ >= kTargetDestroyCount_) {
      // クリアフラグを立てる
      isPhaseCleared_ = true;
      waitTimer_ = kPhaseWaitTime_;
      printf("Attack Cleared! Wait for finish...\n");
    }
    break;

  case TutorialPhase::kComplete:
    // --- 完了画面での選択処理 ---

      isStarted_ = false;

    // 上入力 (選択肢を上に移動)
    // ※ Aボタン判定を削除し、TriggerPadUpなどに変更
    if (Input::GetInstance()->TriggerPadDown(0, XINPUT_GAMEPAD_A) ||
        Input::GetInstance()->TriggerKeyDown(DIK_W) ||
        Input::GetInstance()->TriggerKeyDown(DIK_UP)) {

      if (currentOption_ == CompleteOption::kGoToGame) {
        currentOption_ = CompleteOption::kRetry;
      } else if (currentOption_ == CompleteOption::kBackToTitle) {
        currentOption_ = CompleteOption::kGoToGame;
      }
    }

    // 下入力 (選択肢を下に移動)
    // ※ Aボタン判定を削除し、TriggerPadDownなどに変更
    if (Input::GetInstance()->TriggerPadDown(0, XINPUT_GAMEPAD_A) ||
        Input::GetInstance()->TriggerKeyDown(DIK_S) ||
        Input::GetInstance()->TriggerKeyDown(DIK_DOWN)) {

      if (currentOption_ == CompleteOption::kRetry) {
        currentOption_ = CompleteOption::kGoToGame;
      } else if (currentOption_ == CompleteOption::kGoToGame) {
        currentOption_ = CompleteOption::kBackToTitle;
      }
    }

    // 決定入力 (Aボタン or SPACE)
    if (Input::GetInstance()->TriggerPadDown(0, XINPUT_GAMEPAD_A) ||
        Input::GetInstance()->TriggerKeyDown(DIK_SPACE)) {

      // ここでフェードアウトを開始
      sceneState_ = SceneState::kFadeOut;
      fade_->Start(Fade::Phase::kFadeOut);
    }
    break;
  }
}

void TutorialScene::ResetTutorialState() {
  // 1. プレイヤーとモデルを削除
  player_.reset();
  playerModel_.reset();

  // 2. 障害物リストとモデルリストをクリア
  obstacleSlow_.clear();
  obstacleNormal_.clear();
  obstacleFast_.clear();
  obstacleMax_.clear();

  obstacleSlowModel_.clear();
  obstacleNormalModel_.clear();
  obstacleFastModel_.clear();
  obstacleMaxModel_.clear();

  // 3. 壁とゴールのリストもクリア
  walls_.clear();
  wallModels_.clear();
  goals_.clear();
  goalModels_.clear();

  // ◎ フェーズ内の「達成状況」だけリセットします
  isPhaseCleared_ = false;      // クリア済みフラグを下ろす
  waitTimer_ = 0;               // 演出用タイマーリセット
  destroyedObstaclesCount_ = 0; // 破壊カウントを0に戻す（Attackフェーズ用）

  // ---------------------------------------------------

  // 4. マップ上のオブジェクトを再生成
  GenerateFieldObjects();
}