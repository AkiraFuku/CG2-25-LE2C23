#include "TitleScene.h"
#include "ModelManager.h"
#include "Input.h"
#include "imgui.h"
#include "GameScene.h"
#include "SceneManager.h"
#include "ParticleManager.h"//フレームワークに移植
#include "PSOMnager.h"
#include "LightManager.h"
#include "Fade.h"
void TitleScene::Initialize() {

    LightManager::GetInstance()->AddDirectionalLight({ 1,1,1,1 }, { 0,-1,0 }, 1.0f);
    // カメラの生成と設定
    camera_ = std::make_unique<Camera>();
    camera_->SetTranslate({ 0.0f, 1.5f, -10.0f }); // 少し手前に引く
    camera_->SetRotate({ 0.0f, 0.0f, 0.0f });
    // 自作エンジンの仕組み：描画対象にカメラを登録する（またはCommonにセットする）
    Object3dCommon::GetInstance()->SetDefaultCamera(camera_.get());

    // スカイドーム
    skydome_ = std::make_unique<SkyDome>();
    skydome_->Initialize();

    // タイトルロゴ
    titleObject_ = std::make_unique<Object3d>();
    titleObject_->Initialize();
    titleObject_->SetModel("titleFont/titleFont.obj");
    titleObject_->SetScale({ 0.5f, 0.5f, 0.5f });
    titleObject_->SetTranslate({ 0.0f, posY_, 9.0f });


    // プレイヤー（装飾）
    playerObject_ = std::make_unique<Object3d>();
    playerObject_->Initialize();
    playerObject_->SetModel("player.obj");
    playerObject_->SetScale({ 1.0f, 1.0f, 1.0f });
    playerObject_->SetRotate({ 0.0f, 3.14f * 0.65f, 0.0f }); // Y軸回転
    playerObject_->SetTranslate({ 0.0f, 0.0f, 0.0f });

    Fade::GetInstance()->Start(Fade::Status::FadeIn, 1.0f);
    phase_ = Phase::kFadeIn; // とりあえずMainから開始
}

void TitleScene::Finalize() {
    LightManager::GetInstance()->ClearLights();
}

void TitleScene::Update() {
    // カメラ更新
    camera_->Update();
    XINPUT_STATE state;

    // 現在のジョイスティックを取得
    Input::GetInstance()->GetJoyStick(0, state);
    switch (phase_) {
    case TitleScene::Phase::kFadeIn:
        Fade::GetInstance()->Update();
        if (Fade::GetInstance()->IsFinished()) {
            phase_ = TitleScene::Phase::kMain;
        }
        break;
    case TitleScene::Phase::kMain:
        if (Input::GetInstance()->PushedKeyDown(DIK_SPACE) || Input::GetInstance()->TriggerPadDown(0, XINPUT_GAMEPAD_A)) {
            Fade::GetInstance()->Start(Fade::Status::FadeOut, 1.0f);
            phase_ = TitleScene::Phase::kFadeOut;

        }

        break;


    case TitleScene::Phase::kFadeOut:
        Fade::GetInstance()->Update();
        if (Fade::GetInstance()->IsFinished()) {
            SceneManager::GetInstance()->ChangeScene("GameScene");
            // フェードアウトが終わったら、次のシーンへ
        }
        break;


    }
    // アニメーション計算 
    counter_ += 1.0f / 60.0f;
    counter_ = std::fmod(counter_, kTimeTitleMove);
    float angle = counter_ / kTimeTitleMove * 2.0f * 3.14159f;

    // タイトルの浮遊アニメーション
    Vector3 titlePos = titleObject_->GetTranslate();
    titlePos.y = std::sin(angle) + posY_;
    titleObject_->SetTranslate(titlePos);

    // 更新処理
    skydome_->Update();
    titleObject_->Update();
    playerObject_->Update();


    // シーン遷移処理
    // 自作エンジンのInputクラスに合わせて書き換え
    // 例: Input::GetInstance()->TriggerKey(DIK_SPACE) や TriggerPadButton(...)
    //if (Input::GetInstance()->TriggerPadDown(0, XINPUT_GAMEPAD_A) ||
    //    Input::GetInstance()->TriggerKeyDown(DIK_SPACE)) {

    //    // シーン切り替え予約
    //    SceneManager::GetInstance()->ChangeScene("GameScene");
    //}
}

void TitleScene::Draw() {
    // 描画コマンド発行
    skydome_->Draw();
    titleObject_->Draw();
    playerObject_->Draw();

    Fade::GetInstance()->Draw();
}

TitleScene::~TitleScene()
{
}
