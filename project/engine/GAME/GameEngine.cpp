#include "GameEngine.h"
#include "Fade.h"
#include "StageManager.h"
void GameEngine::Initialize() {

   Framework::Initialize();
  
 
  
   sceneFactory_ = std::make_unique< SceneFactory>();

   
   SceneManager::GetInstance()->SetSceneFactory(sceneFactory_.get());

   SceneManager::GetInstance()->ChangeScene("TitleScene");
     Fade::GetInstance()->Initialize();
     StageManager::GetInstance()->RoadStage();

};
void GameEngine::Finalize() {  
    SceneManager::GetInstance()->Finalize();
    Fade::GetInstance()->Finalize();
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