#pragma once
#include"MathFunction.h"
#include "Sprite.h"
#include"Object3D.h"
#include "Model.h"
#include "Camera.h"
#include "ParicleEmitter.h"
#include "Audio.h"
#include "TextureManager.h"

class TitleScene
{
    public:
    void Initialize();
    void Finalize();
    void Update();
    void Draw();
private:
      Camera* camera;
    Sprite* sprite;
   
  
    Audio::SoundData soundData1;
};

