#pragma once
#include<Windows.h>
#include<filesystem>
#include<strsafe.h>
#include<dbghelp.h>
#pragma comment(lib,"Dbghelp.lib")

#include "WinApp.h"//フレームワークに移動予定
#include "DXCommon.h"//フレームワークに移動予定
#include"Audio.h"//シングルトン化済　フレームワークに移動予定
#include"Input.h"//シングルトン化予定　フレームワークに移動予定
#include"D3DResourceLeakChecker.h"//フレームワークに移動予定
#include"StringUtility.h"//フレームワークに移動予定
#include"Logger.h"//フレームワークに移動予定
#include "SpriteCommon.h"//シングルトン化予定//フレームワークに移動予定
#include "TextureManager.h"//シングルトン化済//フレームワークに移動予定
#include"Object3DCommon.h"//シングルトン化予定//フレームワークに移動予定
#include "ModelCommon.h"//シングルトン化予定//フレームワークに移動予定
#include "ModelManager.h"//シングルトン化済//フレームワークに移動予定
#include "SrvManager.h"//シングルトン化予定 //フレームワークに移動予定
#include "ParticleManager.h"//シングルトン化済 //フレームワークに移動予定
#include "ImGuiManager.h"//シングルトン化予定 //フレームワークに移動予定
class Framwork
{
public:
    virtual ~Framwork() = default;
    virtual void Initialize();
    virtual void Finalize();
    virtual void Update();
    virtual  void Draw();
    virtual  bool IsEnd() {
        return endReqest_;
    }
    void Run();
private:
    bool endReqest_ = false;

     D3DResourceLeakChecker LeakCheck;
    WinApp* winApp;
    DXCommon* dxCommon;
    SrvManager* srvManager;
    ImGuiManager* imguiManager;
    Input* input;
    SpriteCommon* spritecommon;
    Object3dCommon* object3dCommon;

};

