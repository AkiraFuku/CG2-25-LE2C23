#include "GameEngine.h"
//static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception) {
//    //ダンプファイルの作成
//    SYSTEMTIME time;
//    GetLocalTime(&time);
//    wchar_t filePath[MAX_PATH] = { 0 };
//    CreateDirectory(L"./Dumps", nullptr);
//    StringCchPrintfW(filePath, MAX_PATH,
//        L"./Dumps/%04d-%02d%02d-%02d%02d.dmp",
//        time.wYear, time.wMonth, time.wDay,
//        time.wHour, time.wMinute);
//    HANDLE dumpFileHandle = CreateFile(
//        filePath,
//        GENERIC_READ | GENERIC_WRITE,
//        FILE_SHARE_WRITE | FILE_SHARE_READ,
//        0, CREATE_ALWAYS, 0, 0
//    );
//    //プロセスIDとクラッシュが発生したスレッドIDを取得
//    DWORD procesessId = GetCurrentProcessId();
//    DWORD threadId = GetCurrentThreadId();
//    //設定情報入力
//    MINIDUMP_EXCEPTION_INFORMATION minidumpInformation{ 0 };
//    minidumpInformation.ThreadId = threadId;
//    minidumpInformation.ExceptionPointers = exception;
//    minidumpInformation.ClientPointers = TRUE;
//    //ダンプの出力
//    MiniDumpWriteDump(
//        GetCurrentProcess(),
//        procesessId,
//        dumpFileHandle,
//        MiniDumpNormal,
//        &minidumpInformation,
//        nullptr,
//        nullptr
//    );
//    return EXCEPTION_EXECUTE_HANDLER;
//}
//
//std::wstring wstr = L"Hello,DirectX!";
//winmain
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

    //D3DResourceLeakChecker LeakCheck;
    ////D3D12の初期化
    //CoInitializeEx(0, COINIT_MULTITHREADED);

    //SetUnhandledExceptionFilter(ExportDump);
    ////ログ出力用のディレクトリを作成
    //std::filesystem::create_directory("logs");
    //std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    //std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>
    //    nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
    //std::chrono::zoned_time localTime{
    //    std::chrono::current_zone(),
    //    nowSeconds
    //};
    //std::string dataString = std::format(
    //    "{:%Y%m%d_%H%M%S}",
    //    localTime
    //);
    //std::string logFilePath = std::string("logs/") + dataString + ".log";
    ////ファイルへの書き込み
    //std::ofstream logStream(logFilePath);

    //WinApp* winApp = nullptr;

    //winApp = new WinApp();
    //winApp->Initialize();

    //DXCommon* dxCommon = nullptr;
    //dxCommon = new DXCommon();
    //dxCommon->Initialize(winApp);
    //SrvManager* srvManager = nullptr;

    //srvManager = new SrvManager();
    //srvManager->Initialize(dxCommon);


    //ImGuiManager* imguiManager = nullptr;
    //imguiManager = new ImGuiManager();
    //imguiManager->Initialize(dxCommon, srvManager);

    //TextureManager::GetInstance()->Initialize(dxCommon, srvManager);
    //ModelManager::GetInstance()->Initialize(dxCommon);

    //ParticleManager::GetInstance()->Initialize(dxCommon, srvManager);

    //Logger::Log(StringUtility::ConvertString(std::format(L"WSTRING{}\n", wstr)));

    ////ここから書く　外部入力
    //Input* input = nullptr;
    //input = new Input();
    //input->Initialize(winApp);

    //SpriteCommon* spritecommon = nullptr;
    //spritecommon = new SpriteCommon;
    //spritecommon->Initialize(dxCommon);

    //Object3dCommon* object3dCommon = nullptr;
    //object3dCommon = new Object3dCommon;
    //object3dCommon->Initialize(dxCommon);

    //Camera* camera = new Camera();
    //camera->SetRotate({ 0.0f,0.0f,0.0f });
    //camera->SetTranslate({ 0.0f,0.0f,-5.0f });
    //object3dCommon->SetDefaultCamera(camera);

    //ParticleManager::GetInstance()->Setcamera(camera);


    //Audio* audio = new Audio();
    //audio->Initialize();
    //Audio::SoundData soundData1 = Audio::SoundLoadWave("resources/fanfare.mp3");

    //audio->PlayAudio(soundData1);

    //TextureManager::GetInstance()->LoadTexture("resources/uvChecker.png");

    //ParticleManager::GetInstance()->CreateParticleGroup("Test", "resources/uvChecker.png");
    ///*   std::vector<Sprite*> sprites;
    //   for (uint32_t i = 0; i < 5; i++)
    //   {*/

    //Sprite* sprite = new Sprite();
    //// sprite->Initialize(spritecommon,"resources/monsterBall.png");
    //sprite->Initialize(spritecommon, "resources/uvChecker.png");

    //sprite->SetPosition(Vector2{ 25.0f + 100.0f,100.0f });
    //// sprite->SetSize(Vector2{ 100.0f,100.0f });
    ////sprites.push_back(sprite);

    //sprite->SetAnchorPoint(Vector2{ 0.5f,0.5f });

    ////}

    ///* Vector4 color= sprite->GetColor();*/
    // //Vector2 size={};


    //bool fripx = false;
    //bool fripY = false;


    //Object3d* object3d2 = nullptr;
    //object3d2 = new Object3d();
    //object3d2->Initialize(object3dCommon);
    //Object3d* object3d = nullptr;
    //object3d = new Object3d();
    //object3d->Initialize(object3dCommon);


    //ModelManager::GetInstance()->LoadModel("plane.obj");

    //object3d2->SetTranslate(Vector3{ 0.0f,10.0f,0.0f });
    //object3d2->SetModel("axis.obj");
    //object3d->SetModel("plane.obj");
    //Transform M = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
    //ParicleEmitter* emitter = new ParicleEmitter("Test", M, 10, 5.0f, 0.0f);

GameEngine gameEngine;
gameEngine.Initialize();

    //メインループ

    while (true) {
        gameEngine.Update();

//        //メッセージがある限りGetMessageを呼び出す
        if (gameEngine.IsEnd()) {
            break;

        }
//#ifdef USE_IMGUI
//        imguiManager->Begin();
//#endif
//        input->Update();
//
//        emitter->Update();
//        ParticleManager::GetInstance()->Update();
//
//      

        ///////
        ///Update
        ///////


       
//        XINPUT_STATE state;
//
//        // 現在のジョイスティックを取得
//
//
//
//        input->GetJoyStick(0, state);
//
//        // Aボタンを押していたら
//
//        if (input->TriggerPadDown(0, XINPUT_GAMEPAD_A)) {
//
//
//
//            // Aボタンを押したときの処理
//            fripx = !fripx;
//
//
//        }
//        if (input->TriggerPadDown(0, XINPUT_GAMEPAD_B))
//        {
//            fripY = !fripY;
//        }
//
//        //マウスホイールの入力取得
//
//        if (input->GetMouseMove().z)
//        {
//            Vector3 camreaTranslate = camera->GetTranslate();
//            camreaTranslate = Add(camreaTranslate, Vector3{ 0.0f,0.0f,static_cast<float>(input->GetMouseMove().z) * 0.1f });
//            camera->SetTranslate(camreaTranslate);
//
//        }
//        if (input->GetJoyStick(0, state))
//        {
//            // 左スティックの値を取得
//            float x = (float)state.Gamepad.sThumbLX;
//            float y = (float)state.Gamepad.sThumbLY;
//
//            // 数値が大きいので正規化（-1.0 ～ 1.0）して使うのが一般的
//            float normalizedX = x / 32767.0f;
//            float normalizedY = y / 32767.0f;
//            Vector3 camreaTranslate = camera->GetTranslate();
//            camreaTranslate = Add(camreaTranslate, Vector3{ normalizedX/60.0f,normalizedY/60.0f,0.0f });
//            camera->SetTranslate(camreaTranslate);
//        }
//
//
//        
//
//
//
//
//          
//        camera->Update();
//        object3d->Update();
//        object3d2->Update();
//        
//
//#ifdef USE_IMGUI
//        ImGui::Begin("Debug");
//
//        ImGui::Text("Sprite");
//        Vector2 Position =
//            sprite->GetPosition();
//        ImGui::SliderFloat2("Position", &(Position.x), 0.1f, 1000.0f);
//        sprite->SetPosition(Position);
//
//        ImGui::End();
//#endif // USE_IMGUI
//        sprite->SetIsFlipX(fripx);
//        sprite->SetIsFlipY(fripY);
//
//        //sprite->SetRotation(sprite->GetRotation() + 0.1f);
//        sprite->Update();
//
//        //}
//        //ImGui::End();
//        /////////
//        /////Update
//        /////////
//        imguiManager->End();
        /////
        ////DRAW
        /////

       // dxCommon->PreDraw();
       // srvManager->PreDraw();

       // // ParticleManager::GetInstance()->Draw();
       //  // 3Dオブジェクトの描画
       // object3dCommon->Object3dCommonDraw();
       // //object3d2->Draw();
       // object3d->Draw();


       // ///////スプライトの描画

       ///* for (Sprite* sprite : sprites)
       // {*/
       // spritecommon->SpriteCommonDraw();
       // sprite->Draw();
       // /*}*/

       // ///

       // imguiManager->Draw();
       // dxCommon->PostDraw();

       // TextureManager::GetInstance()->ReleaseIntermediateResources();

gameEngine.Draw();
    }

    gameEngine.Finalize();

    //dxCommon->Finalize();

    //delete audio;
    //delete input;
    //delete object3dCommon;
    //delete object3d2;
    //delete object3d;
    //delete camera;
    //imguiManager->Finalize();
    //delete imguiManager;
    ///*   for (Sprite* sprite : sprites)
    //   {*/
    //delete sprite;
    //// }
    //delete emitter;
    //delete spritecommon;
    //delete srvManager;
    //delete dxCommon;
    //dxCommon = nullptr;
    //TextureManager::GetInstance()->Finalize();
    //ModelManager::GetInstance()->Finalize();
    //ParticleManager::GetInstance()->Finalize();

    //winApp->Finalize();
    //delete winApp;
    //winApp = nullptr;

    return 0;
}