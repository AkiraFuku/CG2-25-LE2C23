#include "StageSelectScene.h"
#include "SceneManager.h"
#include <numbers>
#include "Object3dCommon.h"
#include "TextureManager.h"
#include "imgui.h"
#include "LightManager.h"
StageSelectScene::~StageSelectScene() {



    uiSprites_.clear();


}

void StageSelectScene::Initialize() {
    selectStageNo_ = 1;
    Fade::GetInstance()->Start(Fade::Status::FadeIn, 1.0f);
    phase_ = StageSelectScene::Phase::kFadeIn;
    LightManager::GetInstance()->AddDirectionalLight({ 1,1,1,1 }, { 0,-1,0 }, 1.0f);
    // === 追加: スカイドームの初期化 ===
 // カメラの生成と設定
    camera_ = std::make_unique<Camera>();
    camera_->SetTranslate({ 0.0f, 0.0f, -10.0f }); // 少し手前に引く
    camera_->SetRotate({ 0.0f, 0.0f, 0.0f });
    Object3dCommon::GetInstance()->SetDefaultCamera(camera_.get());
    // スカイドーム用モデルの生成 ("skydome" というobjファイルがある前提)


    // スカイドームの生成と初期化
      // スカイドーム
    skydome_ = std::make_unique<SkyDome>();
    skydome_->Initialize();

    // 1. ステージ数などの情報を取得
    int maxStage = (int)StageManager::GetInstance()->GetStageNum();

    // 2. 配列をクリア
    uiSprites_.clear();

    // 3. 選択肢0番: 「タイトルへ戻る」画像
    // ※ファイル名は適宜変更してください
    TextureManager::GetInstance()->LoadTexture("resources/Title.png");
    std::unique_ptr<Sprite> spriteReturn = std::make_unique< Sprite>();
    spriteReturn->Initialize("resources/Title.png");
    spriteReturn->SetAnchorPoint({ 5.0f, 5.0f }); // 中心基準
    spriteReturn->SetSize({ 200.0f,5.0f });
    uiSprites_.push_back(std::move(spriteReturn));

    // 4. 選択肢1番～: 「ステージ数字」画像
    for (int i = 1; i <= maxStage; i++) {
        // ファイル名生成 (例: "Stage1.png", "Stage2.png")
        std::string fileName = "resources/" + std::to_string(i) + ".png";
        TextureManager::GetInstance()->LoadTexture(fileName);

        // スプライトの生成 (new) と初期化
        std::unique_ptr<Sprite> spriteStage = std::make_unique< Sprite>();
        spriteStage->Initialize(fileName);

        // 設定
        spriteStage->SetAnchorPoint({ 0.5f, 0.5f }); // 中心基準

        // リストに追加
        uiSprites_.push_back(std::move(spriteStage));
    }


    // 5. カーソル画像の生成
    TextureManager::GetInstance()->LoadTexture("resources/Cursor.png");
    spriteCursor_ = std::make_unique< Sprite>();
    spriteCursor_->Initialize("resources/Cursor.png");
    spriteCursor_->SetAnchorPoint({ 0.5f, 0.5f });
    spriteCursor_->SetRotation(std::numbers::pi_v<float> / 2.0f);

    // 6. ヘッダー画像の生成（"SELECT STAGE" などのタイトル文字）
    // 画像がない場合はコメントアウトしてください
    // uint32_t texHeader = TextureManager::Load("Header.png");
    // spriteHeader_ = Sprite::Create(texHeader, { WinApp::kWindowWidth / 2.0f, 100.0f });
    // spriteHeader_->SetAnchorPoint({ 0.5f, 0.5f });

}

void StageSelectScene::Update() {
    // StageManagerから最大ステージ数を取得
    const int kMaxStage = (int)StageManager::GetInstance()->GetStageNum(); // intにキャスト
    Input::GetInstance()->GetPreJoyStick(0, prevState_);
    Input::GetInstance()->GetJoyStick(0, state_);
    // --- ステージ選択処理 ---
    if (Input::GetInstance()->GetMouseMove().z)
    {
        Vector3 cameraTranslate = camera_->GetTranslate();
        cameraTranslate = Add(cameraTranslate, Vector3{ 0.0f,0.0f,static_cast<float>(Input::GetInstance()->GetMouseMove().z) * 0.1f });
        camera_->SetTranslate(cameraTranslate);

    }
    camera_->Update();
#ifdef USE_IMGUI

    ImGui::Begin("Stage Select");

    // 【変更】0番なら「タイトルへ」、それ以外ならステージ番号を表示
    if (selectStageNo_ == 0) {
        ImGui::Text("Selection: > Return to Title <"); // タイトル戻る表示
    } else {
        ImGui::Text("Selection: Stage %d", selectStageNo_);
    }

    ImGui::Text("Max Stage: %d", kMaxStage);
    ImGui::Text("Use LEFT/RIGHT to change.");
    ImGui::Text("Press SPACE to decide.");
    ImGui::End();
#endif // DEBUG

    switch (phase_) {
    case StageSelectScene::Phase::kFadeIn:
        Fade::GetInstance()->Update();
        if (Fade::GetInstance()->IsFinished()) {
            Fade::GetInstance()->Stop();
            phase_ = StageSelectScene::Phase::kMain;
        }
        break;

    case StageSelectScene::Phase::kMain:
        // 右キー (ステージ番号を進める)
        if (Input::GetInstance()->TriggerKeyDown(DIK_RIGHT) || (state_.Gamepad.sThumbLX > 20000 && prevState_.Gamepad.sThumbLX <= 20000)) {
            selectStageNo_++;
            // 最大数を超えたら0（タイトル戻る）にする
            if (selectStageNo_ > kMaxStage) {
                selectStageNo_ = 0;
            }
        }

        // 左キー (ステージ番号を戻す)
        if (Input::GetInstance()->TriggerKeyDown(DIK_LEFT) || (state_.Gamepad.sThumbLX < -20000 && prevState_.Gamepad.sThumbLX >= -20000)) {
            selectStageNo_--;
            // 0未満になったら最大ステージにする
            if (selectStageNo_ < 0) {
                selectStageNo_ = kMaxStage;
            }
        }

        // 決定処理
        if (Input::GetInstance()->TriggerKeyDown(DIK_SPACE) || Input::GetInstance()->TriggerPadDown(0, XINPUT_GAMEPAD_A)) {
            // ステージ番号がセットされるが、0の場合は後で判定する
            StageManager::GetInstance()->SetCurrentStage(selectStageNo_);
            Fade::GetInstance()->Start(Fade::Status::FadeOut, 1.0f);
            phase_ = StageSelectScene::Phase::kFadeOut;
        }
        break;

    case StageSelectScene::Phase::kFadeOut:
        Fade::GetInstance()->Update();
        if (Fade::GetInstance()->IsFinished()) {

            // 【修正】0番ならタイトル、それ以外ならゲームへ
            // elseを使わないと両方実行されるバグを防ぎます
            if (selectStageNo_ == 0) {
                SceneManager::GetInstance()->ChangeScene("TitleScene");

            } else {
                SceneManager::GetInstance()->ChangeScene("GameScene");
            }
        }
        break;
    }
    // === スプライトの更新（位置合わせ・アニメーション） ===

    // 画面中心座標
    float centerX = WinApp::kClientWidth / 2.0f;
    float centerY = WinApp::kClientHeight / 2.0f;
    float gapX = 300.0f; // 項目同士の間隔

    // すべての項目を配置
    // 現在選択されている selectStageNo_ が画面中央に来るようにずらして配置するロジック
    for (int i = 0; i < uiSprites_.size(); i++) {
        if (uiSprites_[i] == nullptr) continue;

        // 画面中央からのオフセット（選択番号との差分 × 間隔）
        float offsetX = (float)(i - selectStageNo_) * gapX;

        Vector2 targetPos = { centerX + offsetX, centerY };

        // 位置設定
        uiSprites_[i]->SetPosition(targetPos);

        // 演出：選択中のものは不透明(1.0)、それ以外は半透明(0.5)かつ少し小さくする
        if (i == selectStageNo_) {
            uiSprites_[i]->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
            uiSprites_[i]->SetSize({ 200.0f, 200.0f }); // 画像本来のサイズに合わせて調整
        } else {
            uiSprites_[i]->SetColor({ 0.5f, 0.5f, 0.5f, 1.0f }); // グレーアウト
            uiSprites_[i]->SetSize({ 150.0f, 150.0f }); // 小さく
        }
    }

    // カーソルは中央に固定（あるいは選択アイテムの上に表示）
    if (spriteCursor_) {
        // 少し上に表示
        spriteCursor_->SetPosition({ centerX, centerY - 150.0f });

        // ふわふわさせるアニメーション（お好みで）
        static float floatY = 0;
        floatY += 0.1f;
        spriteCursor_->SetPosition({ centerX, centerY - 150.0f + sinf(floatY) * 10.0f });
    }

    skydome_->Update();
    for (int i = 0; i < uiSprites_.size(); i++) {


        // 位置設定
        uiSprites_[i]->Update();


    }
    spriteCursor_->Update();

}

void StageSelectScene::Draw() {


    skydome_->Draw();

    /*if (spriteHeader_) spriteHeader_->Draw();*/

    // 選択肢（現在の選択以外のものを先に描画し、選択中のものを一番上に描画するときれいです）
    for (int i = 0; i < uiSprites_.size(); i++) {
        if (i != selectStageNo_ && uiSprites_[i]) {
            uiSprites_[i]->Draw();
        }
    }
    // 選択中のものを最後に描画（最前面）
    if (selectStageNo_ >= 0 && selectStageNo_ < uiSprites_.size()) {
        if (uiSprites_[selectStageNo_]) {
            uiSprites_[selectStageNo_]->Draw();
        }
    }

    // カーソル
    if (spriteCursor_) spriteCursor_->Draw();

    Fade::GetInstance()->Draw();


}

void StageSelectScene::Finalize()
{
}
