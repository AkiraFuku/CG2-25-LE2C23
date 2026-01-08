#include "GameScene.h"
#include "ModelManager.h"
#include "Input.h"
#include "imgui.h"
#include "SceneManager.h"
#include "TitleScene.h"
#include "Player.h"
#include "ObstacleSlow.h"

// コンストラクタ
GameScene::GameScene() = default;

// デストラクタ
GameScene::~GameScene() = default;


void GameScene::Initialize() {

    camera = std::make_unique<Camera>();
    camera->SetRotate({ 0.3f,0.0f,0.0f });
    camera->SetTranslate({ 0.0f,5.0f,-20.0f });
    Object3dCommon::GetInstance()->SetDefaultCamera(camera.get());
    ParticleManager::GetInstance()->Setcamera(camera.get());

    soundData1 = Audio::GetInstance()->SoundLoadWave("resources/fanfare.mp3");

    Audio::GetInstance()->PlayAudio(soundData1);

    TextureManager::GetInstance()->LoadTexture("resources/uvChecker.png");

    ParticleManager::GetInstance()->CreateParticleGroup("Test", "resources/uvChecker.png");
    /*   std::vector<Sprite*> sprites;
       for (uint32_t i = 0; i < 5; i++)
       {*/
    sprite = std::make_unique<Sprite>();
    // sprite->Initialize(spritecommon,"resources/monsterBall.png");
    sprite->Initialize("resources/uvChecker.png");

    sprite->SetPosition(Vector2{ 25.0f + 100.0f,100.0f });
    // sprite->SetSize(Vector2{ 100.0f,100.0f });
    //sprites.push_back(sprite);

    sprite->SetAnchorPoint(Vector2{ 0.5f,0.5f });

    //}




   // object3d の初期化
    object3d2 = std::make_unique<Object3d>();
    object3d2->Initialize();
    
    object3d = std::make_unique<Object3d>();
    object3d->Initialize();

    ModelManager::GetInstance()->LoadModel("plane.obj");
    ModelManager::GetInstance()->LoadModel("axis.obj");
    object3d2->SetTranslate(Vector3{ 0.0f,10.0f,0.0f });
    object3d2->SetModel("axis.obj");
    object3d->SetModel("plane.obj");
    Transform M = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
   emitter = std::make_unique<ParicleEmitter>("Test", M, 10, 5.0f, 0.0f);

   // プレイヤーの初期化
   player_ = std::make_unique<Player>();
   playerModel_ = std::make_unique<Object3d>();
   playerModel_->SetTranslate(Vector3{ 0.0f,0.0f,0.0f });
   playerModel_->SetModel("cube.obj");
   playerModel_->Initialize();
   player_->Initialize(playerModel_.get(), camera.get(), Vector3{ 0.0f, 0.0f, 0.0f });

   // 障害物の初期化
   obstacleSlow_ = std::make_unique<ObstacleSlow>();
   obstacleSlowModel_ = std::make_unique<Object3d>();
   obstacleSlowModel_->SetTranslate(Vector3{ 0.0f,0.0f, 0.0f });
   obstacleSlowModel_->SetModel("cube.obj");
   obstacleSlowModel_->Initialize();
   obstacleSlow_->Initialize(obstacleSlowModel_.get(), camera.get(), Vector3{ 0.0f, 0.0f, 40.0f });
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
    if (Input::GetInstance()->TriggerPadDown(0, XINPUT_GAMEPAD_B))
    {

    }

    //マウスホイールの入力取得

    if (Input::GetInstance()->GetMouseMove().z)
    {
        Vector3 camreaTranslate = camera->GetTranslate();
        camreaTranslate = Add(camreaTranslate, Vector3{ 0.0f,0.0f,static_cast<float>(Input::GetInstance()->GetMouseMove().z) * 0.1f });
        camera->SetTranslate(camreaTranslate);

    }
    if (Input::GetInstance()->GetJoyStick(0, state))
    {
        // 左スティックの値を取得
        float x = (float)state.Gamepad.sThumbLX;
        float y = (float)state.Gamepad.sThumbLY;

        // 数値が大きいので正規化（-1.0 ～ 1.0）して使うのが一般的
        float normalizedX = x / 32767.0f;
        float normalizedY = y / 32767.0f;
        Vector3 camreaTranslate = camera->GetTranslate();
        camreaTranslate = Add(camreaTranslate, Vector3{ normalizedX / 60.0f,normalizedY / 60.0f,0.0f });
        camera->SetTranslate(camreaTranslate);
    }

    camera->Update();
    object3d->Update();
    object3d2->Update();

    // プレイヤーの更新処理
    player_->Update();

    // 障害物の更新処理
    obstacleSlow_->Update();

    // 全ての当たり判定を行う
    CheckAllCollisions();



#ifdef USE_IMGUI
    ImGui::Begin("Debug");

    ImGui::Text("Sprite");
    Vector2 Position =
        sprite->GetPosition();
    ImGui::SliderFloat2("Position", &(Position.x), 0.1f, 1000.0f);
    sprite->SetPosition(Position);

    ImGui::End();
#endif // USE_IMGUI

    //sprite->SetRotation(sprite->GetRotation() + 0.1f);
    sprite->Update();
}
void GameScene::Draw() {
    //object3d2->Draw();
    //object3d->Draw();
    //ParticleManager::GetInstance()->Draw();

    // プレイヤーの描画処理
    player_->Draw();

    // 障害物の描画処理
    obstacleSlow_->Draw();

    ///////スプライトの描画
    //sprite->Draw();
}

void GameScene::CheckAllCollisions()
{
#pragma region 自キャラと障害物(ゆっくり)の当たり判定
    // 判定対象1と2の座標
    AABB aabb1, aabb2;

    // 自キャラの座標
    aabb1 = player_->GetAABB();

    // 障害物(ゆっくり)の座標
    aabb2 = obstacleSlow_->GetAABB();

    // 当たり判定
    if (isCollision(aabb1, aabb2))
    {
        // 衝突応答
        obstacleSlow_->OnCollision(player_.get());
    }

#pragma endregion
}

bool GameScene::isCollision(const AABB& aabb1, const AABB& aabb2)
{
    if (aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x && aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y && aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z) {
        return true;
    }

    return false;
}
