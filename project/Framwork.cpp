#include "Framwork.h"
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

void Framwork::Initialize()
{
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

    winApp = nullptr;

    winApp = new WinApp();
    winApp->Initialize();

    dxCommon = nullptr;
    dxCommon = new DXCommon();
    dxCommon->Initialize(winApp);
    srvManager = nullptr;
    srvManager = new SrvManager();
    srvManager->Initialize(dxCommon);


    imguiManager = nullptr;
    imguiManager = new ImGuiManager();
    imguiManager->Initialize(dxCommon, srvManager);


    TextureManager::GetInstance()->Initialize(dxCommon, srvManager);
    ModelManager::GetInstance()->Initialize(dxCommon);

    ParticleManager::GetInstance()->Initialize(dxCommon, srvManager);

    Logger::Log(StringUtility::ConvertString(std::format(L"WSTRING{}\n", wstr)));
    //ここから書く　外部入力
    input = nullptr;
    input = new Input();
    input->Initialize(winApp);

    spritecommon = nullptr;
    spritecommon = new SpriteCommon;
    spritecommon->Initialize(dxCommon);

    object3dCommon = nullptr;
    object3dCommon = new Object3dCommon;
    object3dCommon->Initialize(dxCommon);



    Audio::GetInstance()->Initialize();
}

void Framwork::Update()
{
#ifdef USE_IMGUI
    imguiManager->Begin();
#endif
    input->Update();


    imguiManager->End();
}

void Framwork::Draw()
{
    dxCommon->PreDraw();
    srvManager->PreDraw();
    // ParticleManager::GetInstance()->Draw();
     // 3Dオブジェクトの描画
    object3dCommon->Object3dCommonDraw();
   
    ///////スプライトの描画
    spritecommon->SpriteCommonDraw();
}

void Framwork::Finalize()
{
    dxCommon->Finalize();
    Audio::GetInstance()->Finalize();
    delete input;
    delete object3dCommon;
    imguiManager->Finalize();
    delete imguiManager;

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
}
std::wstring wstr = L"Hello,DirectX!";
void Framwork::Run()
{
    Initialize();
    while (true) {
        Update();
        if (IsEnd())break;
        Draw();
    }
    Finalize();
}
