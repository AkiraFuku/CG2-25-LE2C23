#pragma once
#include<Windows.h>
#include<filesystem>
#include<strsafe.h>
#include<dbghelp.h>
#pragma comment(lib,"Dbghelp.lib")
#pragma comment(lib,"dxguid.lib")


#include "WinApp.h"//フレームワークに移植
#include "DXCommon.h"//フレームワークに移植
#include"Audio.h"//フレームワークに移植
#include"Input.h"//フレームワークに移植
#include"D3DResourceLeakChecker.h"//フレームワークに移植
#include"StringUtility.h"//フレームワークに移植
#include"Logger.h"//フレームワークに移植
#include "SpriteCommon.h"//フレームワークに移植
#include "TextureManager.h"//フレームワークに移植
#include"Object3DCommon.h"//フレームワークに移植
#include "ModelCommon.h"//フレームワークに移植
#include "ModelManager.h"//フレームワークに移植
#include "SrvManager.h"//フレームワークに移植
#include "ParticleManager.h"//フレームワークに移植
#include "ImGuiManager.h"//フレームワークに移植
#include"MathFunction.h"
#include "Sprite.h"
#include"Object3D.h"
#include "Model.h"
#include "Camera.h"
#include "ParicleEmitter.h"
#include "Franework.h"
class GameEngine : public Franework
{
public:
    void Initialize() override;

    void Finalize()override;

    void Update()override;

    void Draw()override;


private:
    D3DResourceLeakChecker LeakCheck;
    WinApp* winApp;
    DXCommon* dxCommon;
    SrvManager* srvManager;
    ImGuiManager* imguiManager;
    Camera* camera;
    Sprite* sprite;
    Object3d* object3d2;
    Object3d* object3d;
    ParicleEmitter* emitter;
    Audio::SoundData soundData1;
private:
    //ログファイルパス
    const std::filesystem::path logFilePath = "D3DResourceLeakLog.txt";
    bool endReqest_ = false;


};

