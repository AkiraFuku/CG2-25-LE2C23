#pragma once
#include<Windows.h>
#include<filesystem>
#include<strsafe.h>
#include<dbghelp.h>
#pragma comment(lib,"Dbghelp.lib")
#pragma comment(lib,"dxguid.lib")


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
class GameEngine
{
public:
    void Initialize();

    void Finalize();

    void Update();

    void Draw();
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
    Audio* audio;
    Sprite* sprite;
    Object3d* object3d2;
    Object3d* object3d;
    ParicleEmitter* emitter;
};

