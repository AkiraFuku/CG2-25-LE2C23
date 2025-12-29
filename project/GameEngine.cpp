#include "GameEngine.h"


void GameEngine::Initialize() {

   Framework::Initialize();
   // //D3D12の初期化
   // CoInitializeEx(0, COINIT_MULTITHREADED);

   // SetUnhandledExceptionFilter(ExportDump);
   // //ログ出力用のディレクトリを作成
   // std::filesystem::create_directory("logs");
   // std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
   // std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>
   //     nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
   // std::chrono::zoned_time localTime{
   //     std::chrono::current_zone(),
   //     nowSeconds
   // };
   // std::string dataString = std::format(
   //     "{:%Y%m%d_%H%M%S}",
   //     localTime
   // );
   // std::string logFilePath = std::string("logs/") + dataString + ".log";
   // //ファイルへの書き込み
   // std::ofstream logStream(logFilePath);

   // winApp = nullptr;

   // winApp = new WinApp();
   // winApp->Initialize();

   // dxCommon = nullptr;
   // dxCommon = new DXCommon();
   // dxCommon->Initialize(winApp);
   // srvManager = nullptr;
   // srvManager = new SrvManager();
   // srvManager->Initialize(dxCommon);


   // imguiManager = nullptr;
   // imguiManager = new ImGuiManager();
   // imguiManager->Initialize(dxCommon, srvManager);

   // TextureManager::GetInstance()->Initialize(dxCommon, srvManager);
   // ModelManager::GetInstance()->Initialize(dxCommon);

   // ParticleManager::GetInstance()->Initialize(dxCommon, srvManager);

   // Logger::Log(StringUtility::ConvertString(std::format(L"WSTRING{}\n", wstr)));

   // //ここから書く　外部入力
   // Input::GetInstance()->Initialize(winApp);

   // SpriteCommon::GetInstance()->Initialize(dxCommon);

   //Object3dCommon::GetInstance()->Initialize(dxCommon);

    camera = new Camera();
    camera->SetRotate({ 0.0f,0.0f,0.0f });
    camera->SetTranslate({ 0.0f,0.0f,-5.0f });
    Object3dCommon::GetInstance()->SetDefaultCamera(camera);

    ParticleManager::GetInstance()->Setcamera(camera);


   /* Audio::GetInstance()->Initialize();*/

    soundData1 = Audio::GetInstance()->SoundLoadWave("resources/fanfare.mp3");

     Audio::GetInstance()->PlayAudio(soundData1);

    TextureManager::GetInstance()->LoadTexture("resources/uvChecker.png");

    ParticleManager::GetInstance()->CreateParticleGroup("Test", "resources/uvChecker.png");
    /*   std::vector<Sprite*> sprites;
       for (uint32_t i = 0; i < 5; i++)
       {*/
    sprite = new Sprite();
    // sprite->Initialize(spritecommon,"resources/monsterBall.png");
    sprite->Initialize( "resources/uvChecker.png");

    sprite->SetPosition(Vector2{ 25.0f + 100.0f,100.0f });
    // sprite->SetSize(Vector2{ 100.0f,100.0f });
    //sprites.push_back(sprite);

    sprite->SetAnchorPoint(Vector2{ 0.5f,0.5f });

    //}




    object3d2 = nullptr;
    object3d2 = new Object3d();
    object3d2->Initialize();
    object3d = nullptr;
    object3d = new Object3d();
    object3d->Initialize();


    ModelManager::GetInstance()->LoadModel("plane.obj");

    object3d2->SetTranslate(Vector3{ 0.0f,10.0f,0.0f });
    object3d2->SetModel("axis.obj");
    object3d->SetModel("plane.obj");
    Transform M = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
    emitter = new ParicleEmitter("Test", M, 10, 5.0f, 0.0f);



};
void GameEngine::Finalize() {
  /*  dxCommon->Finalize();

    Audio::GetInstance()->Finalize();
    Input::GetInstance()->Finalize();
    Object3dCommon::GetInstance()->Finalize();*/
    delete object3d2;
    delete object3d;
    delete camera;
  /*  imguiManager->Finalize();
    delete imguiManager;*/
    /*   for (Sprite* sprite : sprites)
       {*/
    delete sprite;
    // }
    delete emitter;
   /* SpriteCommon::GetInstance()->Finalize();
    delete srvManager;
    delete dxCommon;
    dxCommon = nullptr;
    TextureManager::GetInstance()->Finalize();
    ModelManager::GetInstance()->Finalize();
    ParticleManager::GetInstance()->Finalize();

    winApp->Finalize();
    delete winApp;
    winApp = nullptr;*/

    Framework::Finalize();
};
void GameEngine::Update() {
    Framework::Update();
//    //メッセージがある限りGetMessageを呼び出す
//    if (winApp->ProcessMessage()) {
//        endReqest_ = true;
//        return;
//
//    }
//#ifdef USE_IMGUI
//    imguiManager->Begin();
//#endif
//    Input::GetInstance()->Update();

    emitter->Update();
   /* ParticleManager::GetInstance()->Update();*/

    XINPUT_STATE state;

    // 現在のジョイスティックを取得



    Input::GetInstance()->GetJoyStick(0, state);

    // Aボタンを押していたら

    if (Input::GetInstance()->TriggerPadDown(0, XINPUT_GAMEPAD_A)) {



        // Aボタンを押したときの処理



    }
    if (Input::GetInstance()->TriggerPadDown(0, XINPUT_GAMEPAD_B))
    {

    }

    //マウスホイールの入力取得

    if (Input::GetInstance()->GetMouseMove().z)
    {
        Vector3 camreaTranslate = camera->GetTranslate();
        camreaTranslate = Add(camreaTranslate, Vector3{ 0.0f,0.0f,static_cast<float>(Input::GetInstance()->GetMouseMove().z) * 0.1f });
        camera->SetTranslate(camreaTranslate);

    }
    if (Input::GetInstance()->GetJoyStick(0, state))
    {
        // 左スティックの値を取得
        float x = (float)state.Gamepad.sThumbLX;
        float y = (float)state.Gamepad.sThumbLY;

        // 数値が大きいので正規化（-1.0 ～ 1.0）して使うのが一般的
        float normalizedX = x / 32767.0f;
        float normalizedY = y / 32767.0f;
        Vector3 camreaTranslate = camera->GetTranslate();
        camreaTranslate = Add(camreaTranslate, Vector3{ normalizedX / 60.0f,normalizedY / 60.0f,0.0f });
        camera->SetTranslate(camreaTranslate);
    }

    camera->Update();
    object3d->Update();
    object3d2->Update();


#ifdef USE_IMGUI
    ImGui::Begin("Debug");

    ImGui::Text("Sprite");
    Vector2 Position =
        sprite->GetPosition();
    ImGui::SliderFloat2("Position", &(Position.x), 0.1f, 1000.0f);
    sprite->SetPosition(Position);

    ImGui::End();
#endif // USE_IMGUI

    //sprite->SetRotation(sprite->GetRotation() + 0.1f);
    sprite->Update();

    //}
    //ImGui::End();
    /////////
    /////Update
    /////////
    /*imguiManager->End();*/
};
void GameEngine::Draw() {

   

    // ParticleManager::GetInstance()->Draw();
     // 3Dオブジェクトの描画
   
    //object3d2->Draw();
    object3d->Draw();


    ///////スプライトの描画

   /* for (Sprite* sprite : sprites)
    {*/
    sprite->Draw();
    /*}*/

    Framework::Draw();
    ///


}