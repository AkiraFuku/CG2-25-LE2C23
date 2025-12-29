#pragma once
#include<Windows.h>
#include<filesystem>
#include<strsafe.h>
#include<dbghelp.h>
#pragma comment(lib,"Dbghelp.lib")
#pragma comment(lib,"dxguid.lib")


#include "WinApp.h"//フレームワークに移動予定
#include "DXCommon.h"//フレームワークに移動予定
#include"MathFunction.h"
#include"Audio.h"//シングルトン化済　フレームワークに移動予定
#include"Input.h"//シングルトン化予定　フレームワークに移動予定
#include"D3DResourceLeakChecker.h"//フレームワークに移動予定
#include"StringUtility.h"//フレームワークに移動予定
#include"Logger.h"//フレームワークに移動予定
#include "Sprite.h"
#include "SpriteCommon.h"//シングルトン化予定//フレームワークに移動予定
#include "TextureManager.h"//シングルトン化済//フレームワークに移動予定
#include"Object3DCommon.h"//シングルトン化予定//フレームワークに移動予定
#include"Object3D.h"
#include "ModelCommon.h"//シングルトン化予定//フレームワークに移動予定
#include "Model.h"
#include "ModelManager.h"//シングルトン化済//フレームワークに移動予定
#include "Camera.h"
#include "SrvManager.h"//シングルトン化予定 //フレームワークに移動予定
#include "ParticleManager.h"//シングルトン化済 //フレームワークに移動予定
#include "ParicleEmitter.h"
#include "ImGuiManager.h"//シングルトン化予定 //フレームワークに移動予定
class GameEngine
{
public:
    void Initialize();

    void Finalize();

    void Update();

    void Draw();

    bool IsEnd() {
        return endReqest_;
    }
private:
    D3DResourceLeakChecker LeakCheck;
    WinApp* winApp;
    DXCommon* dxCommon;
    SrvManager* srvManager;
    ImGuiManager* imguiManager;
    Input* input;
    SpriteCommon* spritecommon;
    Object3dCommon* object3dCommon;
    Camera* camera;
    Sprite* sprite;
    Object3d* object3d2;
    Object3d* object3d;
    ParicleEmitter* emitter;
    Audio::SoundData soundData1;
private:
    //ログファイルパス
    const std::filesystem::path logFilePath = "D3DResourceLeakLog.txt";
    //終了フラグ
    bool endReqest_ = false;

};

