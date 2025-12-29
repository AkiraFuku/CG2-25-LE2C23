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

class Framework
{
public:
    virtual ~Framework() = default;
    virtual void Initialize();
    virtual void Finalize();
    virtual void Update();
    virtual void Draw() = 0;
    bool IsEnd() {
        return endReqest_;
    }
    void Run();

    void RequestEnd() {
        endReqest_ = true;
    }
private:
    bool endReqest_ = false;

    D3DResourceLeakChecker LeakCheck;
    WinApp* winApp;
    DXCommon* dxCommon;
    SrvManager* srvManager;
    ImGuiManager* imguiManager;

};

