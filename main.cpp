#include<Windows.h>
#include<cstdint>
#include<string>
//#include<format>
#include<filesystem>
#include<fstream>
#include<chrono>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<strsafe.h>
#include<cassert>
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#include<dbghelp.h>
#pragma comment(lib,"Dbghelp.lib")


void Log(std::ofstream& os, const std::string& message) {
    os << message << std::endl;
    OutputDebugStringA(message.c_str());

}
std::wstring ConvertString(const std::string& str) {
    if (str.empty()) {
        return std::wstring();
    }

    auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
    if (sizeNeeded == 0) {
        return std::wstring();
    }
    std::wstring result(sizeNeeded, 0);
    MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
    return result;
}

std::string ConvertString(const std::wstring& str) {
    if (str.empty()) {
        return std::string();
    }

    auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
    if (sizeNeeded == 0) {
        return std::string();
    }
    std::string result(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
    return result;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
        switch (msg) {
            //
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        
        }
        return DefWindowProc(hwnd, msg, wparam, lparam);
}
static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception){
    //ダンプファイルの作成
    SYSTEMTIME time;
    GetLocalTime(&time);
    wchar_t filePath[MAX_PATH]={0};
    CreateDirectory(L"./Dumps", nullptr);
    StringCchPrintfW(filePath, MAX_PATH,
        L"./Dumps/%04d-%02d%02d-%02d%02d.dmp",
        time.wYear, time.wMonth, time.wDay,
        time.wHour, time.wMinute);
    HANDLE dumpFileHandle = CreateFile(
        filePath,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_WRITE | FILE_SHARE_READ,
        0,CREATE_ALWAYS,0,0
    );
    //プロセスIDとクラッシュが発生したスレッドIDを取得
    DWORD procesessId = GetCurrentProcessId();
    DWORD threadId = GetCurrentThreadId();
    //設定情報入力
    MINIDUMP_EXCEPTION_INFORMATION minidumpInformation{0};
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
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int){
    SetUnhandledExceptionFilter(ExportDump);
    //ログ出力用のディレクトリを作成
    std::filesystem::create_directory("logs");
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::chrono::time_point<std::chrono::system_clock,std::chrono::seconds>
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



    WNDCLASS wc={};
    //ウィンドウプロシージャ
    wc.lpfnWndProc = WindowProc;
    //ウィンドウクラスの名前
    wc.lpszClassName=L"CG2WindowClass";
    //インスタンスハンドル
    wc.hInstance = GetModuleHandle(nullptr);
    //カーソル
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    //ウィンドウクラスの登録
    RegisterClass(&wc);
	// Output
   // Log("Hello,DirectX!\n");
    Log(logStream,ConvertString( std::format( L"WSTRING{}\n",wstr)));

	//OutputDebugStringA("Hello,DirectX!\n");
    // //ウィンドウのサイズ
    const int32_t kClientWidth = 1280;
    const int32_t kClientHeight = 720;

    RECT wrc = { 0, 0, kClientWidth, kClientHeight };

    //  
    AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, FALSE);
    //ウィンドウの作成
      HWND hwnd = CreateWindow(
            wc.lpszClassName,//クラス名
            L"CG2",
         WS_OVERLAPPEDWINDOW,
         CW_USEDEFAULT,
         CW_USEDEFAULT,
         wrc.right - wrc.left,
         wrc.bottom - wrc.top,
         nullptr,
         nullptr,
         wc.hInstance,
         nullptr
     );


                  //ウィンドウを表示
            ShowWindow(hwnd, SW_SHOW);
            // DXGIファクトリーの作成
            IDXGIFactory7* dxgiFactory = nullptr;
            // 
            HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
            //
            assert(SUCCEEDED(hr));

            //アダプターの作成
            IDXGIAdapter4* useAdapter = nullptr;
            //良い順番のアダプターを探す
            for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i,DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                IID_PPV_ARGS(&useAdapter))!=DXGI_ERROR_NOT_FOUND ; ++i){
                ///アダプターの情報を取得
                DXGI_ADAPTER_DESC3 adapterDesc{};
                hr = useAdapter->GetDesc3(&adapterDesc);
                assert(SUCCEEDED(hr));
                if (!(adapterDesc.Flags&DXGI_ADAPTER_FLAG3_SOFTWARE)){
                    Log(logStream,ConvertString(std::format(L"Use Adapter:{}\n",adapterDesc.Description)));
                    break;
                }
                useAdapter=nullptr;
            }
            assert(useAdapter != nullptr);

            ID3D12Device* device = nullptr;

            D3D_FEATURE_LEVEL featureLevels[] ={
                D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0,
            };
            const char* featureLevelStrings[] = {"12_2", "12_1", "12_0"};
            for (size_t i = 0; i < _countof(featureLevels); i++){
                hr = D3D12CreateDevice(
                    useAdapter,
                    featureLevels[i],
                    IID_PPV_ARGS(&device));

                 if (SUCCEEDED(hr)){
                    Log(logStream, (std::format("Use FeatureLevel : {}\n", featureLevelStrings[i])));
                    break;

                 }
            }
            assert(device!=nullptr);
            Log(logStream, "Complete create D3D12Device!!!\n" );
            //コマンドキューの作成
            ID3D12CommandQueue* commandQueue = nullptr;
            D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
            hr = device->CreateCommandQueue(
                &commandQueueDesc,
                IID_PPV_ARGS(&commandQueue));
            assert(SUCCEEDED(hr));
            //コマンドアロケーターの作成
            ID3D12CommandAllocator* commandAllocator = nullptr;
            hr = device->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                IID_PPV_ARGS(&commandAllocator));
            assert(SUCCEEDED(hr));
            //コマンドリストの作成
            ID3D12GraphicsCommandList* commandList = nullptr;
            hr =device->CreateCommandList(
                0,
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                commandAllocator, nullptr,
                IID_PPV_ARGS(&commandList)
            );
            assert(SUCCEEDED(hr));
            //スワップチェーンの作成
            IDXGISwapChain4* swapChain = nullptr;
            DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
            swapChainDesc.Width = kClientWidth;//画像の幅
            swapChainDesc.Height = kClientHeight;//画像の高さ
            swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//色の形式
            swapChainDesc.SampleDesc.Count = 1;//マルチサンプルしない
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//レンダリングターゲットとして使用
            swapChainDesc.BufferCount = 2;//バッファの数
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;//写したら破棄
            // コマンドキュー,ウィンドウハンドル、設定して生成
            hr = dxgiFactory->CreateSwapChainForHwnd(
                commandQueue,
                hwnd,
                &swapChainDesc,
                nullptr,
                nullptr,
                reinterpret_cast<IDXGISwapChain1**>(&swapChain)
            );
            assert(SUCCEEDED(hr));

            //ディスクプリプターヒープの作成
            ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;
            D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc{};
            rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;//レンダリングターゲットビュー
            rtvDescriptorHeapDesc.NumDescriptors = 2;//バッファの数
            hr = device->CreateDescriptorHeap(
                &rtvDescriptorHeapDesc,
                IID_PPV_ARGS(&rtvDescriptorHeap)
            );
            assert(SUCCEEDED(hr));
            //スワップチェーンからリソースをひっぱる
            ID3D12Resource* swapChainResources[2] = {nullptr};
            hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
            assert(SUCCEEDED(hr));
            hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
            assert(SUCCEEDED(hr));
            // RTVの作成
            D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
            rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//出力結果をSRGBに変換・書き込み
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;//2Dテクスチャ
            //ディスクリプタの先頭を取得
            D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
            //ディスクリプタ２つ用意
            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
            //rtv一つ目 先頭を入れる
            rtvHandles[0] = rtvStartHandle;
            device->CreateRenderTargetView(
                swapChainResources[0],
                &rtvDesc,
                rtvHandles[0]
            );
            //rtv二つ目 ハンドルを得る
            rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            //2つ目のRTVを作成
            device->CreateRenderTargetView(
                swapChainResources[1],
                &rtvDesc,
                rtvHandles[1]
            );
      //
      

    MSG msg{};
    while (msg.message != WM_QUIT) {
        //メッセージがある限りGetMessageを呼び出す
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else{



        }
       
    }
 

    
	return 0;
}