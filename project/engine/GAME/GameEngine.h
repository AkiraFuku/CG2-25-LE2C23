#pragma once

#include "Scene.h"
#include "TitleScene.h"
#include "GameScene.h"
#include "SceneManager.h"
#include "Framework.h"
class GameEngine : public Framework
{
public:
    void Initialize() override;

    void Finalize()override;

    void Update()override;

    void Draw()override;


private:
  
  
    SceneManager* sceneManager =nullptr;
  
private:
    //ログファイルパス
    const std::filesystem::path logFilePath = "D3DResourceLeakLog.txt";
   


};

