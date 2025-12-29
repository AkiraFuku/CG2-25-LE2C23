#include "GameEngine.h"


void GameEngine::Initialize() {

   Framework::Initialize();
  
   
  
   sceneFactory_ = new SceneFactory();

   
   SceneManager::GetInstance()->SetSceneFactory(sceneFactory_);

   
   SceneManager::GetInstance()->ChangeScene("TitleScene");

};
void GameEngine::Finalize() {

    delete sceneFactory_;
  
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