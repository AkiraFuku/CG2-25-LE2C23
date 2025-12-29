#pragma once
#include"MathFunction.h"
//#include "Sprite.h"
//#include"Object3D.h"
//#include "Camera.h"
//#include "ParicleEmitter.h"
#include "GamePlayScene.h"
#include "Framework.h"
class GameEngine : public Framework
{
public:
    void Initialize() override;

    void Finalize()override;

    void Update()override;

    void Draw()override;


private:

    GamePlayScene* scene_=nullptr;
   
  
    /*Camera* camera;
    Sprite* sprite;
    Object3d* object3d2;
    Object3d* object3d;
    ParicleEmitter* emitter;
    Audio::SoundData soundData1;*/
private:
    //ログファイルパス
    const std::filesystem::path logFilePath = "D3DResourceLeakLog.txt";
   


};

