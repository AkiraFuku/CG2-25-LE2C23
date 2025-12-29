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
class TitleScene :public Scene
{
    public:
    void Initialize()override;
    void Finalize()override;
    void Update()override;
    void Draw()override;
private:
      Camera* camera;
    Sprite* sprite;
   
  
    Audio::SoundData soundData1;
};

