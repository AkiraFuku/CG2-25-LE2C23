#pragma once
#include "Scene.h"

class SceneManager
{
private:
    // シングルトン用静的インスタンス
    static SceneManager* instance;

    // コンストラクタ・デストラクタをprivateにする
    SceneManager() = default;
    ~SceneManager();

    // コピー禁止
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

public:
    // インスタンス取得
    static SceneManager* GetInstance();

    // 終了処理（インスタンスの破棄）
    void Finalize();

    void SetNextScene(Scene* nextScene) {
        nextScene_ = nextScene;
    }
    void Update();
    void Draw();

private:
    Scene* scene_ = nullptr;
    Scene* nextScene_ = nullptr;
};