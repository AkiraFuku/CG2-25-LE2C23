#pragma once
#include"Object3D.h"
#include "Sprite.h"
#include "Camera.h"
#include "ParicleEmitter.h"
#include "Audio.h"

class GamePlayScene
{

public:
    void Initialize();
    void Finalize();
    void Update();
    void Draw();

private:
    Camera* camera;
    Sprite* sprite;
    Object3d* object3d2;
    Object3d* object3d;
    ParicleEmitter* emitter;
    Audio::SoundData soundData1;

};

