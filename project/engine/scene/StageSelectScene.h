#pragma once
#include "Scene.h"
#include "StageManager.h"
#include "Skydome.h"

#include "Fade.h"
#include "Input.h"
class StageSelectScene :public Scene
{
public:
    ~StageSelectScene() override;
    enum class Phase {
        kFadeIn,  // フェードイン
        kMain,    // メイン
        kFadeOut, // フェードアウト
    };
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;
private:
    // 【変更】計算しやすいように int に変更します
    int selectStageNo_ = 1;


    Phase phase_ = Phase::kFadeIn;

    XINPUT_STATE state_;
    XINPUT_STATE prevState_;

   // スマートポインタで管理
    std::unique_ptr<Camera> camera_;
     std::unique_ptr<SkyDome> skydome_;  // スカイドーム本体
    std::vector< std::unique_ptr<Sprite>> uiSprites_;

    // カーソル（矢印や枠）のスプライト
     std::unique_ptr<Sprite> spriteCursor_  ;

    // タイトル画像（装飾用："STAGE SELECT"などの文字）
 
};


