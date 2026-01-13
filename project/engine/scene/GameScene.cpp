#include "GameScene.h"
#include "ModelManager.h"
#include "Input.h"
#include "imgui.h"
#include "SceneManager.h"
#include "TitleScene.h"
#include "PSOMnager.h"
void GameScene::Initialize() {

    camera = std::make_unique<Camera>();
    camera->SetRotate({ 0.0f,0.0f,0.0f });
    camera->SetTranslate({ 0.0f,0.0f,-5.0f });
    Object3dCommon::GetInstance()->SetDefaultCamera(camera.get());
    ParticleManager::GetInstance()->Setcamera(camera.get());

    handle_ = Audio::GetInstance()->LoadAudio("resources/fanfare.mp3");

    Audio::GetInstance()->PlayAudio(handle_,true);

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
    ModelManager::GetInstance()->CreateSphereModel("MySphere", 16);
    object3d2->SetTranslate(Vector3{ 0.0f,10.0f,0.0f });
    object3d2->SetModel("axis.obj");
    object3d->SetModel("MySphere");
    object3d->SetBlendMode(BlendMode::Add);
    object3d->SetFillMode(FillMode::kSolid);
    Transform M = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
    emitter = std::make_unique<ParicleEmitter>("Test", M, 10, 5.0f, 0.0f);
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
        
        if (Audio::GetInstance()->IsPlaying(handle_))
        {
            
            Audio::GetInstance()->StopAudio(handle_);
        }



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
    /*if (Input::GetInstance()->TriggerMouseDown(0))
    {
        if (!Audio::GetInstance()->IsPlaying(handle_))
        {
            Audio::GetInstance()->PlayAudio(handle_);
        }

    }*/
    if (Input::GetInstance()->TriggerMouseDown(0))
    {
      if (Audio::GetInstance()->IsPlaying(handle_))
        {
            Audio::GetInstance()->PauseAudio(handle_);
      } else
      {
          Audio::GetInstance()->ResumeAudio(handle_);
       
      }
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


#ifdef USE_IMGUI
    ImGui::Begin("Debug");
     ImGui::Text("Sphire");
     Vector3 pos=object3d->GetTranslate();
       ImGui::SliderFloat3("Pos", &(pos.x), 0.1f, 1000.0f);
       object3d->SetTranslate(pos);

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
    object3d2->Draw();
    object3d->Draw();
   // ParticleManager::GetInstance()->Draw();
    ///////スプライトの描画
    sprite->Draw();
}