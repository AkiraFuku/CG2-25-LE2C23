#pragma once
#include"MathFunction.h"
#include "Sprite.h"
#include"Object3D.h"
#include "Model.h"
#include "Camera.h"
#include "ParicleEmitter.h"
#include "Audio.h"
#include "TextureManager.h"
#include "BubbleFade.h"

#include "Scene.h"
#include <memory>
class TitleScene :public Scene
{
public:
    void Initialize()override;
    void Finalize()override;
    void Update()override;
    void Draw()override;
private:
    std::unique_ptr<Camera> camera;
    std::unique_ptr<Sprite> sprite;

    std::unique_ptr<BubbleFade> fade_;
    bool requestedSceneChange_ = false;

    bool isTrigger_ = false;
    float timer_ = 0.0;
    uint32_t count_ = 1;

     uint32_t handle_=0;
};

