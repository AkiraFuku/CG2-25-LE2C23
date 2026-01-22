#pragma once
#include "Sprite.h"
#include <memory>

class Fade {
public:
    enum class Status {
        None,
        FadeIn,
        FadeOut
    };

    // シングルトン
    static Fade* GetInstance();
    void Finalize();

    void Initialize();
    void Update();
    void Draw();

    void Start(Status status, float duration);
    void Stop();
    bool IsFinished() const;

private:
    Fade() = default;
    ~Fade() = default;
    Fade(const Fade&) = delete;
    Fade& operator=(const Fade&) = delete;

    static Fade* instance;

    // 自作エンジンのSpriteを使用
    std::unique_ptr<Sprite> sprite_;

    Status status_ = Status::None;
    float duration_ = 0.0f;
    float counter_ = 0.0f;
};