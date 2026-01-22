#pragma once
#include"MathFunction.h"
#include "Sprite.h"
#include"Object3D.h"
#include "Model.h"
#include "Camera.h"
#include "ParicleEmitter.h"
#include "Audio.h"
#include "TextureManager.h"

#include "Scene.h"
#include <memory>

#include "Skydome.h"
class TitleScene :public Scene
{
public:

    enum class Phase {

		kFadeIn, // フェードイン
		kMain,   // メイン
		kGaid, //ゲーム説明
		kFadeOut, // フェードアウト

	};

    void Initialize()override;
    void Finalize()override;
    void Update()override;
    void Draw()override;
private:
  

    // スマートポインタで管理
    std::unique_ptr<Camera> camera_;
    std::unique_ptr<SkyDome> skydome_;

    // タイトルロゴとプレイヤー（装飾用）
    std::unique_ptr<Object3d> titleObject_;
    std::unique_ptr<Object3d> playerObject_;


    // アニメーション用変数
    float counter_ = 0.0f;
    static inline const float kTimeTitleMove = 2.0f;

    Phase phase_ = Phase::kFadeIn;

     uint32_t handle_=0;
};

