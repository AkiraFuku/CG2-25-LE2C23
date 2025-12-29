#include "GameEngine.h"


void GameEngine::Initialize() {

   Framework::Initialize();
  
    scene = new GameScene();
    scene->Initialize();
};
void GameEngine::Finalize() {
  
    scene->Finalize();
    //delete object3d2;
    //delete object3d;
    //delete camera;
    //delete sprite;
    //delete emitter;
    Framework::Finalize();
};
void GameEngine::Update() {
    Framework::Update();
   
    scene->Update();

 
};
void GameEngine::Draw() {

    //object3d->Draw();


    /////////スプライトの描画

    //sprite->Draw();
  
    scene->Draw();

    Framework::Draw();
    ///


}