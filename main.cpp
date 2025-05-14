#include<Windows.h>
#include<cstdint>
#include<string>
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
#include<dxgidebug.h>
#pragma comment(lib,"dxguid.lib")
#include<dxcapi.h>
#pragma comment(lib,"dxcompiler.lib")
#include"Vector4.h"



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
/// <summary>
/// 
/// </summary>
/// <param name="filePath"></param>
/// <param name="profile"></param>
/// <param name="dxcUtils"></param>
/// <param name="dxcCompiler"></param>
/// <param name="includeHandler"></param>
/// <returns></returns>
IDxcBlob* CompileShader(
// compilerするshaderファイルのパス
const std::wstring& filePath,
//　compilerに使用するprofile
const wchar_t* profile,
//
IDxcUtils* dxcUtils,
IDxcCompiler3* dxcCompiler,
IDxcIncludeHandler* includeHandler,
std::ofstream& os


){
    //hlslファイルの読み込み
    Log(os,ConvertString(std::format(L"Bigin CompileShader, path:{},profiale:{}\n",filePath,profile)));
    IDxcBlobEncoding* shaderSource = nullptr;
    HRESULT hr = dxcUtils->LoadFile(
        filePath.c_str(),
        nullptr,
        &shaderSource
    );
    assert(SUCCEEDED(hr));
    DxcBuffer shaderSoursBuffer{};
    shaderSoursBuffer.Ptr = shaderSource->GetBufferPointer();
    shaderSoursBuffer.Size = shaderSource->GetBufferSize();
    shaderSoursBuffer.Encoding = DXC_CP_ACP;
    //compileする
    LPCWSTR arguments[] = {
        filePath.c_str(),
        L"-E",L"main",
        L"-T",profile,
        L"-Zi",L"-Qembed_debug",
        L"-Od",
        L"-Zpr",
       
    };

    IDxcResult* shaderResult = nullptr;
    hr = dxcCompiler->Compile(
        &shaderSoursBuffer,
        arguments,
        _countof(arguments),
        includeHandler,
        IID_PPV_ARGS(&shaderResult)
    );
    assert(SUCCEEDED(hr));

    //警告・エラー確認
    IDxcBlobUtf8* shaderError = nullptr;
    shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
    if (shaderError != nullptr&&shaderError->GetStringLength()!=0) {
        //エラーがあった場合
        Log(os,shaderError->GetStringPointer());
        assert(false);
    }
    //compile結果をうけとってわたす
    IDxcBlob* shaderBlob = nullptr;
    hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
    assert(SUCCEEDED(hr));
    Log(os,ConvertString(std::format(L"Compile Succeeded, path:{},profiale:{}\n", filePath, profile)));
    //解放
    shaderSource->Release();
    shaderResult->Release();
    return shaderBlob;


};
ID3D12Resource* CreateBufferResource(ID3D12Device* device, size_t sizeInBytes){
    //リソース用ヒープ
            D3D12_HEAP_PROPERTIES uploadHeapProperties{};
            uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;//アップロードヒープ
            //リソース
            D3D12_RESOURCE_DESC resourceDesc{};
            //バッファリソース
            resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            resourceDesc.Width = sizeInBytes;//リソースのサイズ
            // バッファのサイズ
            resourceDesc.Height = 1;
            resourceDesc.DepthOrArraySize = 1;
            resourceDesc.MipLevels = 1;
            resourceDesc.SampleDesc.Count = 1;
            //
            resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            //リソースを作る
            ID3D12Resource* resource = nullptr;
           HRESULT hr = device->CreateCommittedResource(
                &uploadHeapProperties,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&resource)
            );
            assert(SUCCEEDED(hr));
            return resource;


};





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

#ifdef _DEBUG

            //デバッグレイヤーの有効化
            ID3D12Debug1* debugController = nullptr;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))){
                debugController->EnableDebugLayer();
                //デバッグレイヤーの詳細な情報を取得
                debugController->SetEnableGPUBasedValidation(TRUE);

            }
          
#endif // _DEBUG


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
#ifdef _DEBUG
            ID3D12InfoQueue* infoQueue = nullptr;
            if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue))))
            {
                ///深刻なエラーを出力・停止
                infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
                ///エラーを出力・停止
                infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
                ///警告を出力/停止
                infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
                ///解放
                infoQueue->Release();
                //メッセージID
                D3D12_MESSAGE_ID denyIds[]={
                    D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE,

                };
                //
                D3D12_MESSAGE_SEVERITY severities[]={D3D12_MESSAGE_SEVERITY_INFO};
                D3D12_INFO_QUEUE_FILTER filter{};
                filter.DenyList.NumIDs = _countof(denyIds);
                filter.DenyList.pIDList = denyIds;
                filter.DenyList.NumSeverities = _countof(severities);
                filter.DenyList.pSeverityList = severities;
                //
                infoQueue->PushStorageFilter(&filter);

            }
           

#endif // _DEBUG

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

      //fenceのさくせい
            ID3D12Fence* fence = nullptr;
            uint64_t fenceValue = 0;
            hr = device->CreateFence(
                fenceValue,
                D3D12_FENCE_FLAG_NONE,
                IID_PPV_ARGS(&fence)
            );
            assert(SUCCEEDED(hr));
            //スワップチェーンのフリップを待つためのイベント
            //イベントオブジェクトの作成
            HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
            assert(fenceEvent != nullptr);
            ///

            //DXCCompileの初期化
            IDxcUtils* dxcUtils = nullptr;
            IDxcCompiler3* dxcCompiler = nullptr;
            hr = DxcCreateInstance(
                CLSID_DxcUtils,
                IID_PPV_ARGS(&dxcUtils)
            );
            assert(SUCCEEDED(hr));
            hr = DxcCreateInstance(
                CLSID_DxcCompiler,
                IID_PPV_ARGS(&dxcCompiler)
            );
            assert(SUCCEEDED(hr));

            //includeに対応する為の設定
            IDxcIncludeHandler* includeHandler = nullptr;
            hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
            assert(SUCCEEDED(hr));
            ///

            // RootSignatureの作成
            D3D12_ROOT_SIGNATURE_DESC descriptionRootSignatur{};
            descriptionRootSignatur.Flags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

            ///ルートパラメータの設定
            D3D12_ROOT_PARAMETER rootParameters[1]{};
            rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
            rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//ピクセルシェーダーで使う
            rootParameters[0].Descriptor.ShaderRegister = 0;//シェーダーのレジスタ番号0とバインド
            descriptionRootSignatur.pParameters = rootParameters;//ルートパラメータの設定
            descriptionRootSignatur.NumParameters = _countof(rootParameters);//ルートパラメータの数



            //シリアライズしてバイナリにする;
            ID3DBlob* signatureBlob = nullptr;
            ID3DBlob* errorBlob = nullptr;
            hr = D3D12SerializeRootSignature(
                &descriptionRootSignatur,
                D3D_ROOT_SIGNATURE_VERSION_1,
                &signatureBlob,
                &errorBlob
            );
            if (FAILED(hr)){
                Log(logStream,reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
                assert(false);
            }
            //バイナリを元にルートシグネチャー生成
            ID3D12RootSignature* rootSignature = nullptr;
            hr = device->CreateRootSignature(
                0,
                signatureBlob->GetBufferPointer(),
                signatureBlob->GetBufferSize(),
                IID_PPV_ARGS(&rootSignature)
            );
            assert(SUCCEEDED(hr));
            




            //InputLayoutの設定
            D3D12_INPUT_ELEMENT_DESC inputElementDescs[1] = {};
            inputElementDescs[0].SemanticName = "POSITION";
            inputElementDescs[0].SemanticIndex = 0;
            inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
            inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
            D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
            inputLayoutDesc.pInputElementDescs = inputElementDescs;
            inputLayoutDesc.NumElements = _countof(inputElementDescs);
            ///
            // BlendStateの設定
            D3D12_BLEND_DESC blendDesc{};
            blendDesc.RenderTarget[0].RenderTargetWriteMask=
            D3D12_COLOR_WRITE_ENABLE_ALL;
            //RasteriwrStateの設定
            D3D12_RASTERIZER_DESC rasterizerDesc{};
            rasterizerDesc.CullMode=D3D12_CULL_MODE_BACK;
            rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
            //shaderのコンパイル
            IDxcBlob* vertexShaderBlob = CompileShader(
                L"Object3D.vs.hlsl",
                L"vs_6_0",
                dxcUtils,
                dxcCompiler,
                includeHandler,
                logStream
            );
            assert(vertexShaderBlob != nullptr);

            IDxcBlob* pixelShaderBlob = CompileShader(
                L"Object3D.ps.hlsl",
                L"ps_6_0",
                dxcUtils,
                dxcCompiler,
                includeHandler,
                logStream
            );
            assert(pixelShaderBlob != nullptr);
            //PSOの生成
            D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicPipelineStateDesc{};
            graphicPipelineStateDesc.pRootSignature = rootSignature;
            graphicPipelineStateDesc.InputLayout = inputLayoutDesc;
            graphicPipelineStateDesc.VS ={ vertexShaderBlob->GetBufferPointer(),
            vertexShaderBlob->GetBufferSize() };//ヴァーテックスシェーダー
            graphicPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
            pixelShaderBlob->GetBufferSize() };//ピクセルシェーダー
            graphicPipelineStateDesc.BlendState = blendDesc;//ブレンドステート
            graphicPipelineStateDesc.RasterizerState = rasterizerDesc;//ラスタライザーステート
            ///巻き込むRTV
            graphicPipelineStateDesc.NumRenderTargets = 1;
            graphicPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            ///トポロジー
            graphicPipelineStateDesc.PrimitiveTopologyType = 
            D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            //カラー
            graphicPipelineStateDesc.SampleDesc.Count = 1;
            graphicPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
            //PSOの生成
            ID3D12PipelineState* graphicsPipelineState = nullptr;
            hr = device->CreateGraphicsPipelineState(
                &graphicPipelineStateDesc,
                IID_PPV_ARGS(&graphicsPipelineState)
            );
            assert(SUCCEEDED(hr));
            ///

            ID3D12Resource* vertexResource = CreateBufferResource(device, sizeof(Vector4) * 3);
            ///
            //頂点バッファビューの設定
            D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
            //リソース先頭アドレス
            vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
            //リソースのサイズ
            vertexBufferView.SizeInBytes = sizeof(Vector4) * 3;
            vertexBufferView.StrideInBytes = sizeof(Vector4);

            //頂点データの設定
            Vector4* vertexData = nullptr;
            //書き込む為のアドレス
            vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
            //データの設定
            vertexData[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
            vertexData[1] = { 0.0f, 0.5f, 0.0f, 1.0f };
            vertexData[2] = { 0.5f, -0.5f, 0.0f, 1.0f };
            //ビューポート
            D3D12_VIEWPORT viewport{};
            //
            viewport.Width = static_cast<float>(kClientWidth);
            viewport.Height = static_cast<float>(kClientHeight);
            viewport.TopLeftX = 0.0f;
            viewport.TopLeftY = 0.0f;
            viewport.MinDepth = 0.0f;
            viewport.MaxDepth = 1.0f;

            //シザー矩形
            D3D12_RECT scissorRect{};
            scissorRect.left = 0;
            scissorRect.right = kClientWidth;
            scissorRect.top = 0;
            scissorRect.bottom = kClientHeight;
            ///

            ///マテリアルリソース
            ID3D12Resource* materialResource = CreateBufferResource(device, sizeof(Vector4) );
            //マテリアルデータの設定
            Vector4* materialData = nullptr;
            //書き込む為のアドレス
            materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
            //データの設定
            *materialData =  Vector4(1.0f, 0.0f, 0.0f, 1.0f );

            //コマンドリストの初期化
            




    MSG msg{};
    while (msg.message != WM_QUIT) {
        //メッセージがある限りGetMessageを呼び出す
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else{
            ////

            //backBufferIndexを取得
            UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();
            ///バリアーの設定
            D3D12_RESOURCE_BARRIER barrier{};
            //Transitionバリアー
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            //noneにする
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            //バリアを得るリソース。バックアップｂufferのインデックスを取得
            barrier.Transition.pResource = swapChainResources[backBufferIndex];
            //遷移前（現在）のリソース状態
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
            //遷移後のリソース状態
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
            //transitionバリアーを張る
            commandList->ResourceBarrier(1, &barrier);
            ///
            commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], FALSE, nullptr);
            //
            //クリアカラー
            float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f };
            //
            commandList->ClearRenderTargetView(
                rtvHandles[backBufferIndex],
                clearColor,
                0,
                nullptr
            );
            ///
            commandList->RSSetViewports(1, &viewport);//ビューポートの設定
            commandList->RSSetScissorRects(1, &scissorRect);//シザー矩形の設定
            // RootSignatureの設定
            commandList->SetGraphicsRootSignature(rootSignature);
            //PSOの設定
            commandList->SetPipelineState(graphicsPipelineState);
            //VBVの設定
            commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
            //形状の設定
            commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            //
            commandList->DrawInstanced(3, 1, 0, 0);




            ///
            
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
            //transitionバリアーを張る
            commandList->ResourceBarrier(1, &barrier);
            ///
            //コマンドリストをクローズ
            hr = commandList->Close();
            assert(SUCCEEDED(hr));
            //GPUにコマンドリストを実行
            ID3D12CommandList* commandLists[] = { commandList };
            commandQueue->ExecuteCommandLists(1, commandLists);
            //スワップチェーンをフリップ
            swapChain->Present(1, 0);
            ///gpuの完了を待つ
            fenceValue++;
            commandQueue->Signal(fence, fenceValue);
            ///
            if (fence->GetCompletedValue()<fenceValue)
            {

                fence->SetEventOnCompletion(fenceValue, fenceEvent);
                WaitForSingleObject(fenceEvent, INFINITE);
            }
            //コマンドアロケーターをリセット
            hr = commandAllocator->Reset();
            assert(SUCCEEDED(hr));
            //コマンドリストをリセット
            hr = commandList->Reset(commandAllocator, nullptr);
            assert(SUCCEEDED(hr));
            /////


        }
       
    }
    //リソースの解放
    CloseHandle(fenceEvent);
    fence->Release();
 //RTVの解放
    rtvDescriptorHeap->Release();
//スワップチェーンの解放

    swapChainResources[0]->Release();
    swapChainResources[1]->Release();
    //スワップチェーンの解放
    swapChain->Release();
    //コマンドリストの解放
    commandList->Release();
    commandAllocator->Release();
    //コマンドキューの解放
    commandQueue->Release();
    //デバイスの解放
    device->Release();
    //アダプターの解放
    useAdapter->Release();
    //DXGIファクトリーの解放
    dxgiFactory->Release();
    
    dxcCompiler->Release();
    dxcUtils->Release();
    //includeHandlerの解放
    includeHandler->Release();
    //ルートシグネチャの解放
    rootSignature->Release();
    //シリアライズしたバイナリの解放
    signatureBlob->Release();
    // //頂点リソースの解放
    vertexResource->Release();
    //
    graphicsPipelineState->Release();
    //
    signatureBlob->Release();
    //エラーログの解放
    if (errorBlob != nullptr) {
        errorBlob->Release();
    }
    //シェーダーの解放
    rootSignature->Release();
    vertexShaderBlob->Release();
    pixelShaderBlob->Release();


    




    

    //デバッグレイヤーの解放
#ifdef _DEBUG
    debugController->Release();
#endif
    CloseWindow(hwnd);

    IDXGIDebug1* debug;
    if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
        debug->ReportLiveObjects(DXGI_DEBUG_ALL,DXGI_DEBUG_RLO_ALL);
        debug->ReportLiveObjects(DXGI_DEBUG_APP,DXGI_DEBUG_RLO_ALL);
        debug->ReportLiveObjects(DXGI_DEBUG_D3D12,DXGI_DEBUG_RLO_ALL);

        debug->Release();
    }
 

    
	return 0;
}