#include "Fade.h"
#include "WinApp.h"    // 画面サイズ取得用
#include <algorithm>   // std::clamp
#include <cmath>

Fade* Fade::instance = nullptr;

Fade* Fade::GetInstance() {
    if (instance == nullptr) {
        instance = new Fade();
    }
    return instance;
}

void Fade::Finalize() {
    delete instance;
    instance = nullptr;
}

void Fade::Initialize() {
    // スプライトの生成
    sprite_ = std::make_unique<Sprite>();
    
    // ★重要: フェード用に使う白い画像を読み込んでください
    // 色を(0,0,0)にすることで黒フェードになります
    sprite_->Initialize("resources/white1x1.png"); 

    // 画面全体を覆うサイズに設定
    // 自作エンジンのWinAppからサイズを取得
    Vector2 screenSize = { 
        static_cast<float>(WinApp::kClientWidth), 
        static_cast<float>(WinApp::kClientHeight) 
    };
    sprite_->SetSize(screenSize); // SpriteクラスにSetSizeを作る必要があるかもしれません
    
    // 原点を左上に
    sprite_->SetPosition({ 0.0f, 0.0f });
    
    // 最初は透明にしておく
    sprite_->SetColor({ 0.0f, 0.0f, 0.0f, 0.0f }); 
}

void Fade::Start(Status status, float duration) {
    status_ = status;
    duration_ = duration;
    counter_ = 0.0f;
}

void Fade::Stop() {
    status_ = Status::None;
}

bool Fade::IsFinished() const {
    return status_ == Status::None;
}

void Fade::Update() {
    if (status_ == Status::None) return;

    counter_ += 1.0f / 60.0f; // 60FPS想定
    if (counter_ > duration_) {
        counter_ = duration_;
    }

    // アルファ値（透明度）の計算 0.0(透明) 〜 1.0(不透明)
    float alpha = 0.0f;

    if (status_ == Status::FadeIn) {
        // 暗転状態(1.0)から透明(0.0)へ
        alpha = 1.0f - (counter_ / duration_);
    }
    else if (status_ == Status::FadeOut) {
        // 透明(0.0)から暗転状態(1.0)へ
        alpha = counter_ / duration_;
    }

    // 終了判定
    if (counter_ >= duration_) {
        status_ = Status::None;
    }

    // 色をセット (黒色 + 計算したアルファ値)
    sprite_->SetColor({ 0.0f, 0.0f, 0.0f, std::clamp(alpha, 0.0f, 1.0f) });
    
    // 更新処理（行列計算など）
    sprite_->Update();
}

void Fade::Draw() {
    // フェード中のみ描画
    // あるいはアルファが0より大きければ描画
    if (sprite_ && sprite_->GetColor().w > 0.0f) {
        sprite_->Draw();
    }
}