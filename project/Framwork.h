#pragma once
#include<Windows.h>
#include<filesystem>
#include<strsafe.h>
#include<dbghelp.h>
#pragma comment(lib,"Dbghelp.lib")

#include "WinApp.h"
#include "DXCommon.h"
#include"Audio.h"
#include"Input.h"//シングルトン化予定　フレームワークに移動予定
#include"D3DResourceLeakChecker.h"
#include"StringUtility.h"
#include"Logger.h"
#include "SpriteCommon.h"//シングルトン化予定//フレームワークに移動予定
#include "TextureManager.h"
#include"Object3DCommon.h"//シングルトン化予定//フレームワークに移動予定
#include "ModelCommon.h"
#include "ModelManager.h"
#include "SrvManager.h"
#include "ParticleManager.h"
#include "ImGuiManager.h"
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

