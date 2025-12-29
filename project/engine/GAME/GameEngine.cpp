#include "GameEngine.h"


void GameEngine::Initialize() {

   Framework::Initialize();
  
   
  
   Scene* firstScene = new TitleScene();
   SceneManager::GetInstance()->SetNextScene(firstScene);

   

};
void GameEngine::Finalize() {
  
    SceneManager::GetInstance()->Finalize();
   
    Framework::Finalize();
};
void GameEngine::Update() {
    Framework::Update();
   
    SceneManager::GetInstance()->Update();
    ParticleManager::GetInstance()->Update();
 
};
void GameEngine::Draw() {

   
  
    SceneManager::GetInstance()->Draw();

    Framework::Draw();
    ///


}