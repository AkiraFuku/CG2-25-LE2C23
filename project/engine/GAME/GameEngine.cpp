#include "GameEngine.h"


void GameEngine::Initialize() {

   Framework::Initialize();
  
    scene = new GameScene();
    scene->Initialize();
};
void GameEngine::Finalize() {
  
    scene->Finalize();
   
    Framework::Finalize();
};
void GameEngine::Update() {
    Framework::Update();
   
    scene->Update();

 
};
void GameEngine::Draw() {

   
  
    scene->Draw();

    Framework::Draw();
    ///


}