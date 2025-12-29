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
class GameScene :public Scene
{
public:
    void Initialize() override;
    void Finalize()override;
    void Update()override;
    void Draw()override;
private:
      Camera* camera;
    Sprite* sprite;
    Object3d* object3d2;
    Object3d* object3d;
    ParicleEmitter* emitter;
    Audio::SoundData soundData1;
};

