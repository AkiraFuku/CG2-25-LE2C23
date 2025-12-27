#include<Windows.h>
#include<filesystem>
#include<strsafe.h>
#include<dbghelp.h>
#pragma comment(lib,"Dbghelp.lib")
//#include<dxgidebug.h>
//#pragma comment(lib,"dxguid.lib")
//#include<dxcapi.h>
//#pragma comment(lib,"dxcompiler.lib")


#include "WinApp.h"
#include "DXCommon.h"
#include"MathFunction.h"
#include"Audio.h"
#include"Input.h"
#include"D3DResourceLeakChecker.h"
#include"StringUtility.h"
#include"Logger.h"
#include "Sprite.h"
#include "SpriteCommon.h"
#include "TextureManager.h"
#include"Object3DCommon.h"
#include"Object3D.h"
#include "ModelCommon.h"
#include "Model.h"
#include "ModelManager.h"
#include "Camera.h"
#include "SrvManager.h"
#include "ParticleManager.h"
#include "ParicleEmitter.h"
#include "ImGuiManager.h"



static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception) {
    //ダンプファイルの作成
    SYSTEMTIME time;
    GetLocalTime(&time);
    wchar_t filePath[MAX_PATH] = { 0 };
    CreateDirectory(L"./Dumps", nullptr);
    StringCchPrintfW(filePath, MAX_PATH,
        L"./Dumps/%04d-%02d%02d-%02d%02d.dmp",
        time.wYear, time.wMonth, time.wDay,
        time.wHour, time.wMinute);
    HANDLE dumpFileHandle = CreateFile(
        filePath,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_WRITE | FILE_SHARE_READ,
        0, CREATE_ALWAYS, 0, 0
    );
    //プロセスIDとクラッシュが発生したスレッドIDを取得
    DWORD procesessId = GetCurrentProcessId();
    DWORD threadId = GetCurrentThreadId();
    //設定情報入力
    MINIDUMP_EXCEPTION_INFORMATION minidumpInformation{ 0 };
    minidumpInformation.ThreadId = threadId;
    minidumpInformation.ExceptionPointers = exception;
    minidumpInformation.ClientPointers = TRUE;
    //ダンプの出力
    MiniDumpWriteDump(
        GetCurrentProcess(),
        procesessId,
        dumpFileHandle,
        MiniDumpNormal,
        &minidumpInformation,
        nullptr,
        nullptr
    );
    return EXCEPTION_EXECUTE_HANDLER;
}

std::wstring wstr = L"Hello,DirectX!";
//winmain
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

    D3DResourceLeakChecker LeakCheck;
    //D3D12の初期化
    CoInitializeEx(0, COINIT_MULTITHREADED);

    SetUnhandledExceptionFilter(ExportDump);
    //ログ出力用のディレクトリを作成
    std::filesystem::create_directory("logs");
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>
        nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
    std::chrono::zoned_time localTime{
        std::chrono::current_zone(),
        nowSeconds
    };
    std::string dataString = std::format(
        "{:%Y%m%d_%H%M%S}",
        localTime
    );
    std::string logFilePath = std::string("logs/") + dataString + ".log";
    //ファイルへの書き込み
    std::ofstream logStream(logFilePath);

    WinApp* winApp = nullptr;

    winApp = new WinApp();
    winApp->Initialize();

    DXCommon* dxCommon = nullptr;
    dxCommon = new DXCommon();
    dxCommon->Initialize(winApp);

    

    SrvManager* srvManager=nullptr;
    srvManager=new SrvManager();
    srvManager->Initialize(dxCommon);


    ImGuiManager* imguiManager = nullptr;
    imguiManager = new ImGuiManager();
    imguiManager->Initialize(dxCommon,srvManager);

    

    TextureManager::GetInstance()->Initialize(dxCommon,srvManager);
    ModelManager::GetInstance()->Initialize(dxCommon);

    ParticleManager::GetInstance()->Initialize(dxCommon,srvManager);

    Logger::Log(StringUtility::ConvertString(std::format(L"WSTRING{}\n", wstr)));

    //ここから書く　外部入力
    Input* input = nullptr;
    input = new Input();
    input->Initialize(winApp);

    SpriteCommon* spritecommon = nullptr;
    spritecommon = new SpriteCommon;
    spritecommon->Initialize(dxCommon);

    Object3dCommon* object3dCommon = nullptr;
    object3dCommon = new Object3dCommon;
    object3dCommon->Initialize(dxCommon);

    Camera* camera = new Camera();
    camera->SetRotate({ 0.0f,0.0f,0.0f });
    camera->SetTranslate({ 0.0f,0.0f,-5.0f });
    object3dCommon->SetDefaultCamera(camera);

     ParticleManager::GetInstance()->Setcamera(camera);


    Audio* audio = new Audio();
    audio->Initialize();
    Audio::SoundData soundData1 = Audio::SoundLoadWave("resources/fanfare.wav");

    audio->PlayAudio(soundData1);

    TextureManager::GetInstance()->LoadTexture("resources/uvChecker.png");

    ParticleManager::GetInstance()->CreateParticleGroup("Test","resources/uvChecker.png");
    std::vector<Sprite*> sprites;
    for (uint32_t i = 0; i < 5; i++)
    {

        Sprite* sprite = new Sprite();
        // sprite->Initialize(spritecommon,"resources/monsterBall.png");
        sprite->Initialize(spritecommon, "resources/uvChecker.png");

        sprite->SetPosition(Vector2{ i * 25.0f + 100.0f,100.0f });
        // sprite->SetSize(Vector2{ 100.0f,100.0f });
        sprites.push_back(sprite);

        sprite->SetAnchorPoint(Vector2{ 0.5f,0.5f });

    }

    /* Vector4 color= sprite->GetColor();*/
     //Vector2 size={};


    bool fripx = false;
    bool fripY = false;


    Object3d* object3d2 = nullptr;
    object3d2 = new Object3d();
    object3d2->Initialize(object3dCommon);
    Object3d* object3d = nullptr;
    object3d = new Object3d();
    object3d->Initialize(object3dCommon);


    ModelManager::GetInstance()->LoadModel("plane.obj");

    object3d2->SetTranslate(Vector3{ 0.0f,10.0f,0.0f });
    object3d2->SetModel("axis.obj");
    object3d->SetModel("plane.obj");
Transform M={{1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f}};
    ParicleEmitter *emitter=new ParicleEmitter("Test",M,10,5.0f,0.0f);

    //メインループ

    while (true) {


        //メッセージがある限りGetMessageを呼び出す
        if (winApp->ProcessMessage()) {
            break;

        }

        imguiManager->Begin();

        input->Update();

        emitter->Update();
        ParticleManager::GetInstance()->Update();

        //// キー入力判定
       //if (input->TriggerKeyDown(DIK_D)) {
        //    OutputDebugStringA("DIK_D\n");
        //    transform.rotate.y += 0.1f; // 右に移動
        //}
        //if (input->TriggerKeyDown(DIK_A))
        //{
        //    OutputDebugStringA("DIK_A\n");
        //    transform.rotate.y -= 0.1f; // 左に移動
        //}

        ///////
        ///Update
        ///////

       // 

      /* Vector2 pos=sprite->GetPosition();
        pos+=Vector2{0.1f,0.1f};
        sprite->SetPosition(pos);*/

        /* float rotat=sprite->GetRotation();
         rotat+=0.1f;
         sprite->SetRotation(rotat);*/



       // ImGui::Begin("MaterialData");
        //Vector3 camreaTranslate = camera->GetTranslate();
        //camreaTranslate=Add(camreaTranslate,Vector3{0.0f,0.0f,-0.5f});
        ////Vector3 cameraRotate = camera->GetRotate();
        //////ImGui::DragFloat3("Camera Transrate", &(camreaTranslate.x));
        //////ImGui::DragFloat3("Camera rotateate", &(cameraRotate.x));
        ////camera->SetRotate(cameraRotate);
        //camera->SetTranslate(camreaTranslate);
        //ImGui::ColorEdit4("Color", &(materialData->color).x); 
        //bool enableLighting = materialData->enableLighting != 0; // Convert int32_t to bool
        //ImGui::Checkbox("enable", &enableLighting);
        //materialData->enableLighting = enableLighting; // Update the original value after modification
        //ImGui::DragFloat3("rotate",&(transform.rotate.x));
        //ImGui::DragFloat3("traslate", &(transform.traslate.x));
        //ImGui::Checkbox("useMonsterBall",&useMonstorBall);

       /* color=sprite->GetColor();
        ImGui::ColorEdit4("ColorSprite", &(color).x);
        sprite->SetColor(color);*/

        /* Vector2 size=sprite->GetSize();
          size.x+=0.1f;
          size.y+=0.1f;
         sprite->SetSize(size);*/

         //ImGui::DragFloat3("traslateSprite",&(transformSprite.traslate.x));
       /*  ImGui::ColorEdit4("LightColor", &(directionalLightData->color).x);
         ImGui::DragFloat3("Light Direction", &(directionalLightData->direction.x));
         ImGui::InputFloat("intensity",&(directionalLightData->intensity));*/

         /* ImGui::DragFloat2("uvTransformSprite", &uvTransformSprite.traslate.x,0.01f,-10.0f,10.0f);
          ImGui::DragFloat2("uvScaleSprite", &uvTransformSprite.scale.x,0.01f,-10.0f,10.0f);
          ImGui::SliderAngle("uvRotateSprite", &uvTransformSprite.rotate.z);*/

          /* Matrix4x4 cameraMatrix = MakeAfineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.traslate);
           Matrix4x4 viewMatrix = Inverse(cameraMatrix);
           Matrix4x4 worldMatrix = MakeAfineMatrix(transform.scale,transform.rotate,transform.traslate);
           wvpData->WVP = Multiply(worldMatrix, Multiply(viewMatrix,projectionMatirx));
           wvpData->World=worldMatrix;
              directionalLightData->direction= Normalize(directionalLightData->direction);*/

              /* Matrix4x4 uvTransformMatrix=MakeScaleMatrix(uvTransformSprite.scale);
               uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
               uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.traslate));*/
               //materialDataSprite->uvTransform = uvTransformMatrix;
        camera->Update();
        object3d->Update();
        object3d2->Update();
       /* ImGui::Text("Sprite");
        ImGui::Checkbox("fripX", &(fripx));
        ImGui::Checkbox("fripy", &(fripY));*/


        for (Sprite* sprite : sprites)
        {


            sprite->SetIsFlipX(fripx);
            sprite->SetIsFlipY(fripY);

            //sprite->SetRotation(sprite->GetRotation() + 0.1f);
            sprite->Update();

        }
        //ImGui::End();
        // 
        imguiManager->End();
        /////////
        /////Update
        /////////

        /////
        ////DRAW
        /////
        
        dxCommon->PreDraw();
        srvManager->PreDraw();

       // ParticleManager::GetInstance()->Draw();
        // 3Dオブジェクトの描画
        object3dCommon->Object3dCommonDraw();
        object3d2->Draw();
        object3d->Draw();


        ///////スプライトの描画

        //for (Sprite* sprite : sprites)
        //{
        //    spritecommon->SpriteCommonDraw();
        //    sprite->Draw();

        //}

        ///

        imguiManager->Draw();
        dxCommon->PostDraw();

        TextureManager::GetInstance()->ReleaseIntermediateResources();

    }
    imguiManager->Finalize();
    dxCommon->Finalize();

    delete audio;
    delete input;
    delete object3dCommon;
    delete object3d2;
    delete object3d;
    delete camera;
    delete imguiManager;

    for (Sprite* sprite : sprites)
    {
        delete sprite;
    }
    delete emitter;
    delete spritecommon;
    delete srvManager;
    delete dxCommon;
    dxCommon = nullptr;
    TextureManager::GetInstance()->Finalize();
    ModelManager::GetInstance()->Finalize();
    ParticleManager::GetInstance()->Finalize();

    winApp->Finalize();
    delete winApp;
    winApp = nullptr;

    return 0;
}