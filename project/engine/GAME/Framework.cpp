#include "Framework.h"
#include "ParticleManager.h"//フレームワークに移植

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
void Framework::Initialize()
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

    winApp = std::make_unique<WinApp>();
    winApp->Initialize();

    dxCommon = std::make_unique<DXCommon>();
    // 引数には生のポインタが必要なので .get() を使用
    dxCommon->Initialize(winApp.get());
    srvManager = std::make_unique<SrvManager>();
    srvManager->Initialize(dxCommon.get());


    imguiManager = std::make_unique<ImGuiManager>();
    imguiManager->Initialize(dxCommon.get(), srvManager.get());
    TextureManager::GetInstance()->Initialize(dxCommon.get(), srvManager.get());
    ModelManager::GetInstance()->Initialize(dxCommon.get());
    ParticleManager::GetInstance()->Initialize(dxCommon.get(), srvManager.get());
    Logger::Log(StringUtility::ConvertString(std::format(L"WSTRING{}\n", wstr)));

    // 外部入力
    Input::GetInstance()->Initialize(winApp.get());

    SpriteCommon::GetInstance()->Initialize(dxCommon.get());
    Object3dCommon::GetInstance()->Initialize(dxCommon.get());
    Audio::GetInstance()->Initialize();


}

void Framework::Finalize()
{
    dxCommon->Finalize();

    Audio::GetInstance()->Finalize();
    Input::GetInstance()->Finalize();
    Object3dCommon::GetInstance()->Finalize();

    imguiManager->Finalize();
   
    SpriteCommon::GetInstance()->Finalize();
    TextureManager::GetInstance()->Finalize();
    ModelManager::GetInstance()->Finalize();
    ParticleManager::GetInstance()->Finalize();

    winApp->Finalize();
}
void Framework::Update()
{
    //メッセージがある限りGetMessageを呼び出す
    if (winApp->ProcessMessage()) {
        endReqest_ = true;


    }
#ifdef USE_IMGUI
    imguiManager->Begin();
#endif
    Input::GetInstance()->Update();


    dxCommon->PreDraw();
    srvManager->PreDraw();
    // SpriteCommon::GetInstance()->SpriteCommonDraw();
    // Object3dCommon::GetInstance()->Object3dCommonDraw();
}

void Framework::Draw()
{
    imguiManager->End();
    imguiManager->Draw();
    dxCommon->PostDraw();
    TextureManager::GetInstance()->ReleaseIntermediateResources();

    // 基底クラスの描画処理（純粋仮想関数として宣言されているため、必ずオーバーライドする必要があります）
}
void Framework::Run()
{
    Initialize();
    while (true) {
        Update();
        if (IsEnd()) {
            break;
        }
        Draw();
    }
    Finalize();
}
