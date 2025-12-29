#pragma once

#include "TitleScene.h"
#include "GameScene.h"
#include "Framework.h"
class GameEngine : public Framework
{
public:
    void Initialize() override;

    void Finalize()override;

    void Update()override;

    void Draw()override;


private:
  
    TitleScene* scene = nullptr;
  
private:
    //ログファイルパス
    const std::filesystem::path logFilePath = "D3DResourceLeakLog.txt";
   


};

