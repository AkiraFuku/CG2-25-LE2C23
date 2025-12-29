#pragma once
#include"Scene.h"


class SceneManager
{
public:
    ~ SceneManager();
    void SetNextScene(Scene* nextScene) {
        nextScene_ = nextScene;
    }
    void Update();
    void Draw();

private:
    Scene* scene_ = nullptr;
    Scene* nextScene_ = nullptr;
};

