#include "TitleScene.h"
#include "ModelManager.h"
#include "Input.h"
#include "imgui.h"
#include "GameScene.h"
#include "SceneManager.h"
#include "ParticleManager.h"//フレームワークに移植
#include "PSOMnager.h"

void TitleScene::Initialize() {

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
    sprite->Initialize("resources/monsterBall.png");

    sprite->SetPosition(Vector2{ 25.0f + 100.0f,100.0f });
    // sprite->SetSize(Vector2{ 100.0f,100.0f });
    //sprites.push_back(sprite);
   // sprite->SetBlendMode(BlendMode::Add);
    sprite->SetAnchorPoint(Vector2{ 0.5f,0.5f });

    //}






    ModelManager::GetInstance()->LoadModel("plane.obj");

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
}
void TitleScene::Finalize() {
  if (fade_) {
    fade_->Finalize();
  }

    ParticleManager::GetInstance()->ReleaseParticleGroup("Test");
  ParticleManager::GetInstance()->Finalize();
}
void TitleScene::Update() {


    XINPUT_STATE state;

    // 現在のジョイスティックを取得
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


    Input::GetInstance()->GetJoyStick(0, state);

    if (Input::GetInstance()->TriggerKeyDown(DIK_SPACE)) {
      if (!fade_->IsActive()) {
        requestedSceneChange_ = true;
        fade_->StartFadeOut();
      }
    }

    fade_->Update();

    // 覆い完了したらシーン切替（ここがキモ）
    if (requestedSceneChange_ && fade_->IsCoverComplete()) {
      // ここでScene切替
      GetSceneManager()->ChangeScene("GameScene");

      // ※GameScene側で fade_->StartFadeIn() したいなら、
      // FadeをSceneManager側に持たせるか、Fadeを共有オブジェクトにすると綺麗。
      // TitleScene内にFadeがある設計なら、TitleSceneでChangeSceneした瞬間に消えるので
      // 「フェードインもやりたい」なら Fade を
      // SceneManager/Framework側に置くのが正解。
    }

    // Aボタンを押していたら

    if (Input::GetInstance()->TriggerPadDown(0, XINPUT_GAMEPAD_A)) {



        // Aボタンを押したときの処理

        if (Audio::GetInstance()->IsPlaying(handle_))
        {
            
            Audio::GetInstance()->StopAudio(handle_);
        }

        GetSceneManager()->ChangeScene("GameScene");

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
void TitleScene::Draw() {

    if (fade_) {
    fade_->Draw();
  }


    //ParticleManager::GetInstance()->Draw();
    /////////スプライトの描画
    //sprite->Draw();
}