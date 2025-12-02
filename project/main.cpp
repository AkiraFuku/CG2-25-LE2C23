#include<Windows.h>
#include<cstdint>
#include<iostream>
#include<string>
#include<filesystem>
#include<fstream>
#include<chrono>
#include<strsafe.h>
#include<dbghelp.h>
#pragma comment(lib,"Dbghelp.lib")
#include<dxgidebug.h>
#pragma comment(lib,"dxguid.lib")
#include<dxcapi.h>
#include "engine/base/WinApp.h"
#include "engine/base/DXCommon.h"
#pragma comment(lib,"dxcompiler.lib")
#include"engine/math/MassFunction.h"
#include"externals/imgui/imgui.h"
#include"externals/imgui/imgui_impl_dx12.h"
#include"externals/imgui/imgui_impl_win32.h"
#include"externals/DirectXTex/DirectXTex.h"
#include"externals/DirectXTex/d3dx12.h"

#include"engine/audio/Audio.h"
#include "engine/input/Input.h"
#include"engine/base/D3DResourceLeakChecker.h"
#include"StringUtility.h"
#include"Logger.h"

#include "engine/2d/Sprite.h"
#include "engine/2d/SpriteCommon.h"
#include "engine/2d/TextureManager.h"
#include "Data.h"

#include"engine/3d/Object3DCommon.h"
#include"engine/3d/Object3D.h"


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);





LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
    {
        return true;
    }

    switch (msg) {
        //
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}
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

//MaterialData LoadMaterialTemplateFile(const std::string& directryPath, const std::string& filename) {
//
//    //1. 変数の宣言
//
//    MaterialData materialData;
//    std::string line;
//    std::ifstream file(directryPath + "/" + filename);//ファイルパスを結合して開く
//    //2. ファイルを開く
//    assert(file.is_open());//ファイルが開けたか確認
//    //3. ファイルからデータを読み込みマテリアルデータを作成
//    while (std::getline(file, line)) {
//        std::string identifier;
//        std::istringstream s(line);
//        s >> identifier;//行の先頭を識別子として取得
//
//        if (identifier == "map_Kd") {
//
//            std::string textureFileName;
//            s >> textureFileName;//テクスチャファイル名を読み込み
//            //テクスチャのパスを設定
//            materialData.textureFilePath = directryPath + "/" + textureFileName;
//        }
//    }
//
//    //4. マテリアルデータを返す
//    return materialData;
//}
//ModelData LoadObjFile(const std::string& directryPath, const std::string& filename)
//{
//    //1. 変数の宣言
//    ModelData modelData;
//    std::vector<Vector4> positions;//頂点座標
//    std::vector<Vector3> normals;//法線ベクトル
//    std::vector<Vector2> texcoords;//テクスチャ座標
//    std::string line;
//    //2. ファイルを開く
//    std::ifstream file(directryPath + "/" + filename);//ファイルパスを結合して開く
//    assert(file.is_open());//ファイルが開けたか確認
//
//    //3. ファイルからデータを読み込みモデルデータを作成
//    while (std::getline(file, line)) {
//        std::string identifier;
//        std::istringstream s(line);
//        s >> identifier;//行の先頭を識別子として取得
//        ///
//        if (identifier == "v") {
//            Vector4 position;
//            s >> position.x >> position.y >> position.z;//頂点座標を読み込み
//            position.w = 1.0f; // w成分を1.0に設定
//            position.x *= -1.0f; // X軸を反転
//            positions.push_back(position);//頂点座標を追加
//        } else if (identifier == "vt") {
//            Vector2 texcoord;
//            s >> texcoord.x >> texcoord.y;//テクスチャ座標を読み込み
//            // OpenGLとDirectXでY軸の方向が異なるため、Y座標を反転
//            texcoord.y = 1.0f - texcoord.y;
//            texcoords.push_back(texcoord);//テクスチャ座標を追加
//        } else if (identifier == "vn") {
//            Vector3 normal;
//            s >> normal.x >> normal.y >> normal.z;//法線ベクトルを読み込み
//            normal.x *= -1.0f; // X軸を反転
//            normals.push_back(normal);
//
//        } else if (identifier == "f") {
//            VertexData Triangle[3];
//            //面は三角形限定
//            for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
//                std::string vertexDefinition;
//                s >> vertexDefinition;//頂点定義を読み込み
//                //頂点の要素へのIndexは「位置/UV/法線」の順番で格納されているので、分解して取得
//                std::istringstream v(vertexDefinition);
//                uint32_t elementIndices[3];//位置、UV、法線のインデックス
//                for (uint32_t element = 0; element < 3; ++element) {
//                    std::string index;
//                    std::getline(v, index, '/');//スラッシュで区切って取得
//                    elementIndices[element] = std::stoi(index);//文字列を整数に変換
//                }
//                // 要素のインデックスから頂点データを構築
//                Vector4 position = positions[elementIndices[0] - 1];//1から始まるので-1
//                Vector2 texcoord = texcoords[elementIndices[1] - 1];//1から始まるので-1
//                Vector3 normal = normals[elementIndices[2] - 1];//1から始まるので-1
//
//                //    VertexData vertex = { position, texcoord, normal };//頂点データを構築
//                //    modelData.vertices.push_back(vertex);//モデルデータに頂点を追加
//                Triangle[faceVertex] = { position, texcoord, normal };//頂点データを構築
//            }
//            modelData.vertices.push_back(Triangle[2]);
//            modelData.vertices.push_back(Triangle[1]);
//            modelData.vertices.push_back(Triangle[0]);
//        } else if (identifier == "mtllib")//マテリアルライブラリの読み込み
//        {
//            std::string materialFileName;
//            s >> materialFileName;//マテリアルファイル名を読み込み
//            //マテリアルデータを読み込む
//            modelData.material = LoadMaterialTemplateFile(directryPath, materialFileName);
//        }
//
//    }
//    //4. モデルデータを返す
//    return modelData;
//
//}













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
    TextureManager::GetInstance()->Initialize(dxCommon);

    Logger::Log(StringUtility::ConvertString(std::format(L"WSTRING{}\n", wstr)));
    /*HRESULT hr;*/



    //ここから書く　外部入力
    Input* input = nullptr;
    input = new Input();
    input->Initialize(winApp);


    SpriteCommon* spritecommon=nullptr;
    spritecommon= new SpriteCommon;
    spritecommon->Initialize(dxCommon);

    Object3dCommon* object3dCommon = nullptr;
    object3dCommon = new Object3dCommon;
    object3dCommon->Initialize(dxCommon);


    Audio* audio = new Audio();
    audio->Initialize();
    Audio::SoundData soundData1 = Audio::SoundLoadWave("resources/fanfare.wav");

    audio->PlayAudio(soundData1);





    TextureManager::GetInstance()->LoadTexture("resources/uvChecker.png");

    std::vector<Sprite*> sprites;
    for (uint32_t i = 0; i < 5; i++)
    {

        Sprite* sprite = new Sprite();
       // sprite->Initialize(spritecommon,"resources/monsterBall.png");
        sprite->Initialize(spritecommon,"resources/uvChecker.png");

        sprite->SetPosition(Vector2{i*25.0f+100.0f,100.0f});
       // sprite->SetSize(Vector2{ 100.0f,100.0f });
        sprites.push_back(sprite);

        sprite->SetAnchorPoint(Vector2{ 0.5f,0.5f });
        
    }
  
   /* Vector4 color= sprite->GetColor();*/
    //Vector2 size={};
    
           
     bool fripx=false;
     bool fripY=false;


     Object3d* object3d = nullptr;
     object3d = new Object3d();
     object3d->Initialize(object3dCommon);
    






    //メインループ

    while (true) {


        //メッセージがある限りGetMessageを呼び出す
        if (winApp->ProcessMessage()) {
            break;

        }

        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        input->Update();



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
       
       

       ImGui::Begin("MaterialData");
       //ImGui::DragFloat3("Camera Transrate",&(cameraTransform.traslate.x));
       //ImGui::DragFloat3("Camera rotateate",&(cameraTransform.rotate.x));
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

       object3d->Update();
               ImGui::Text("Sprite");
               ImGui::Checkbox("fripX", &(fripx));
               ImGui::Checkbox("fripy",&(fripY));


           for( Sprite*sprite:sprites)
           {
              
               
               sprite->SetIsFlipX(fripx);
               sprite->SetIsFlipY(fripY);
              
               //sprite->SetRotation(sprite->GetRotation() + 0.1f);
               sprite->Update();

           }
 ImGui::End();
        ///////
        ///Update
        ///////


        ///
        //DRAW
        ///
        ImGui::Render();
        dxCommon->PreDraw();
        // 3Dオブジェクトの描画
        object3dCommon->Object3dCommonDraw();
        object3d->Draw();
       

        /////スプライトの描画

        
       
        for (Sprite* sprite: sprites)
        {
            spritecommon->SpriteCommonDraw();
            sprite->Draw();

        }



        ///

        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList().Get());

        dxCommon->PostDraw();

        TextureManager::GetInstance()->ReleaseIntermediateResources();
       
    }



    dxCommon->Finalize();

    delete audio;
    delete input;
    delete object3dCommon;
    delete object3d;
      for (Sprite* sprite: sprites)
        {
          
           delete sprite;

        }
    delete spritecommon;
    delete dxCommon;
    dxCommon = nullptr;
    TextureManager::GetInstance()->Finalize();

    winApp->Finalize();
    delete winApp;
    winApp = nullptr;


    return 0;
}