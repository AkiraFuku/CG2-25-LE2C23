#include "TitleScene.h"
#include "ModelManager.h"
#include "Input.h"
#include "imgui.h"
#include "GameScene.h"
#include "SceneManager.h"
#include "ParticleManager.h"//フレームワークに移植
#include "PSOMnager.h"

void TitleScene::Initialize() {

   
    // カメラの生成と設定
    camera_ = std::make_unique<Camera>();
    camera_->SetTranslate({0.0f, 0.0f, -10.0f}); // 少し手前に引く
    camera_->SetRotate({0.0f, 0.0f, 0.0f});
    // ParticleManager::GetInstance()->Setcamera(camera_)

    // 自作エンジンの仕組み：描画対象にカメラを登録する（またはCommonにセットする）
    Object3dCommon::GetInstance()->SetDefaultCamera(camera_.get());

    // スカイドーム
    skydome_ = std::make_unique<SkyDome>();
    skydome_->Initialize();

    // タイトルロゴ
    titleObject_ = std::make_unique<Object3d>();
    titleObject_->Initialize();
    titleObject_->SetModel("titleFont/titleFont.obj");
    titleObject_->SetScale({20.0f, 20.0f, 20.0f});
    titleObject_->SetTranslate({0.0f, 10.0f, 0.0f});

    // プレイヤー（装飾）
    playerObject_ = std::make_unique<Object3d>();
    playerObject_->Initialize();
    playerObject_->SetModel("player.obj");
    playerObject_->SetScale({1.0f, 1.0f, 1.0f});
    playerObject_->SetRotate({0.0f, 3.14f * 0.65f, 0.0f}); // Y軸回転
    playerObject_->SetTranslate({0.0f, 0.0f, 0.0f});
    

    phase_ = Phase::kMain; // とりあえずMainから開始
}

void TitleScene::Finalize() {
    // unique_ptrを使っているので自動解放されますが、
    // 音声の停止などが必要ならここに書きます
}

void TitleScene::Update() {
    // カメラ更新
    camera_->Update();
    XINPUT_STATE state;

    // 現在のジョイスティックを取得



    Input::GetInstance()->GetJoyStick(0, state);
  if (Input::GetInstance()->GetJoyStick(0, state))
    {
        {
            // 左スティックの値を取得
            float x = (float)state.Gamepad.sThumbLX;
            float y = (float)state.Gamepad.sThumbLY;

            // 数値が大きいので正規化（-1.0 ～ 1.0）して使うのが一般的
            float normalizedX = x / 32767.0f;
            float normalizedY = y / 32767.0f;
            Vector3 cameraTranslate = camera_->GetTranslate();
            cameraTranslate = Add(cameraTranslate, Vector3{ normalizedX / 60.0f,normalizedY / 60.0f,0.0f });
            camera_->SetTranslate(cameraTranslate);
        }
      
    }
    // アニメーション計算 
    counter_ += 1.0f / 60.0f;
    counter_ = std::fmod(counter_, kTimeTitleMove);
    float angle = counter_ / kTimeTitleMove * 2.0f * 3.14159f;
    
    // タイトルの浮遊アニメーション
    Vector3 titlePos = titleObject_->GetTranslate();
    titlePos.y = std::sin(angle) + 10.0f; 
    titleObject_->SetTranslate(titlePos);

    // 更新処理
    skydome_->Update();
    titleObject_->Update();
    playerObject_->Update();


    // シーン遷移処理
    // 自作エンジンのInputクラスに合わせて書き換え
    // 例: Input::GetInstance()->TriggerKey(DIK_SPACE) や TriggerPadButton(...)
    if (Input::GetInstance()->TriggerPadDown(0,XINPUT_GAMEPAD_A) || 
        Input::GetInstance()->TriggerKeyDown(DIK_SPACE)) {
        
        // シーン切り替え予約
        SceneManager::GetInstance()->ChangeScene("GameScene");
    }
}

void TitleScene::Draw() {
    // 描画コマンド発行
    skydome_->Draw();
    titleObject_->Draw();
    playerObject_->Draw();
}