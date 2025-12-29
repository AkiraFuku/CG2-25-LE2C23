#include "GameEngine.h"


void GameEngine::Initialize() {

   Framework::Initialize();
  
   
   sceneManager = new SceneManager();
   Scene* firstScene = new TitleScene();
   sceneManager->SetNextScene(firstScene);

   

};
void GameEngine::Finalize() {
  
    delete sceneManager;
   
    Framework::Finalize();
};
void GameEngine::Update() {
    Framework::Update();
   
    sceneManager->Update();
    ParticleManager::GetInstance()->Update();
 
};
void GameEngine::Draw() {

   
  
    sceneManager->Draw();

    Framework::Draw();
    ///


}