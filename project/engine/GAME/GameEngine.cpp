#include "GameEngine.h"


void GameEngine::Initialize() {

   Framework::Initialize();
  
    scene_ = new GamePlayScene();
    scene_->Initialize();
  /*  camera = new Camera();
    camera->SetRotate({ 0.0f,0.0f,0.0f });
    camera->SetTranslate({ 0.0f,0.0f,-5.0f });
    Object3dCommon::GetInstance()->SetDefaultCamera(camera);

    ParticleManager::GetInstance()->Setcamera(camera);

    soundData1 = Audio::GetInstance()->SoundLoadWave("resources/fanfare.mp3");

     Audio::GetInstance()->PlayAudio(soundData1);*/

    //TextureManager::GetInstance()->LoadTexture("resources/uvChecker.png");

    //ParticleManager::GetInstance()->CreateParticleGroup("Test", "resources/uvChecker.png");
    ///*   std::vector<Sprite*> sprites;
    //   for (uint32_t i = 0; i < 5; i++)
    //   {*/
    //sprite = new Sprite();
    //// sprite->Initialize(spritecommon,"resources/monsterBall.png");
    //sprite->Initialize( "resources/uvChecker.png");

    //sprite->SetPosition(Vector2{ 25.0f + 100.0f,100.0f });
    //// sprite->SetSize(Vector2{ 100.0f,100.0f });
    ////sprites.push_back(sprite);

    //sprite->SetAnchorPoint(Vector2{ 0.5f,0.5f });

    ////}




    //object3d2 = nullptr;
    //object3d2 = new Object3d();
    //object3d2->Initialize();
    //object3d = nullptr;
    //object3d = new Object3d();
    //object3d->Initialize();


    //ModelManager::GetInstance()->LoadModel("plane.obj");

    //object3d2->SetTranslate(Vector3{ 0.0f,10.0f,0.0f });
    //object3d2->SetModel("axis.obj");
    //object3d->SetModel("plane.obj");
    //Transform M = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
    //emitter = new ParicleEmitter("Test", M, 10, 5.0f, 0.0f);
};
void GameEngine::Finalize() {
  
  /*  delete object3d2;
    delete object3d;
    delete camera;
    delete sprite;
    delete emitter;*/
    Framework::Finalize();
};
void GameEngine::Update() {
    Framework::Update();
   

 
};
void GameEngine::Draw() {

    
  

    Framework::Draw();
    ///


}