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
#include"MassFunction.h"
#include"externals/imgui/imgui.h"
#include"externals/imgui/imgui_impl_dx12.h"
#include"externals/imgui/imgui_impl_win32.h"
#include"externals/DirectXTex/DirectXTex.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);



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

    if (ImGui_ImplWin32_WndProcHandler(hwnd,msg,wparam,lparam))
    {
        return true;
    }

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
ID3D12DescriptorHeap* CreateDescriptorHeap( ID3D12Device* device,D3D12_DESCRIPTOR_HEAP_TYPE heepType,UINT numDescriptors,bool shaderVisible)
{
    //ディスクリプタヒープの設定
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
    heapDesc.NumDescriptors = numDescriptors;
    heapDesc.Type = heepType;
    heapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ID3D12DescriptorHeap* descriptorHeap = nullptr;
    HRESULT hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap));
    assert(SUCCEEDED(hr));
    return descriptorHeap;
}

DirectX::ScratchImage LoadTexture(const std::string& filePath)
{
    //テクスチャの読み込み
    DirectX::ScratchImage image{};
    std::wstring filePathW= ConvertString(filePath);
    HRESULT hr = DirectX::LoadFromWICFile(
        filePathW.c_str(),
        DirectX::WIC_FLAGS_FORCE_SRGB,
        nullptr,
        image

    );
    assert(SUCCEEDED(hr));
    //ミップマップの生成
    DirectX::ScratchImage mipImages{};
    hr = DirectX::GenerateMipMaps(
        image.GetImages(),
        image.GetImageCount(),
        image.GetMetadata(),
        DirectX::TEX_FILTER_SRGB,
        0,
        mipImages
    );
    assert(SUCCEEDED(hr));



    return mipImages;


}

ID3D12Resource* CresteTextureResourse(ID3D12Device* device , const DirectX::TexMetadata& metadata)
{
    ///metadataを基にリソースを作成
    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Width = UINT(metadata.width);//幅
    resourceDesc.Height = UINT(metadata.height);//高さ
    resourceDesc.MipLevels = UINT16(metadata.mipLevels);//ミップマップの数    
    resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize);//配列の数
    resourceDesc.Format = metadata.format;//フォーマット
    resourceDesc.SampleDesc.Count = 1;//サンプル数
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);//リソースの次元
    //利用するheapの設定
    D3D12_HEAP_PROPERTIES heapProperties = {};
    heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;//カスタムヒープ
    heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;//CPUのページプロパティ
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;//メモリプールの設定
    //リソースの生成
    ID3D12Resource* resource = nullptr;
    HRESULT hr = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&resource)
    );
    assert(SUCCEEDED(hr));
    return resource;


}

void UploadTextureData(ID3D12Resource* textur,const DirectX::ScratchImage& mipImages)
{
    //Metadataの取得
    const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
    //全mipmapに対して
    for (size_t mipLevel = 0; mipLevel < metadata.mipLevels; ++mipLevel){
        //MipMapLevelを指定して画像を取得
        const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);
        //textureに転送
        HRESULT hr= textur->WriteToSubresource(
            UINT(mipLevel),
            nullptr,//全領域へコピー
            img->pixels,////転送するデータ
            UINT(img->rowPitch),////1行分のバイト数
            UINT(img->slicePitch)////1枚分のバイト数
        );
        assert(SUCCEEDED(hr));
    }
}

ID3D12Resource* CreateDepthStencilTextureResourse(ID3D12Device* device, int32_t width, int32_t height){
    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Width = width;//幅
    resourceDesc.Height = height;//高さ
    resourceDesc.MipLevels = 1;//ミップマップの数
    resourceDesc.DepthOrArraySize = 1;//配列の数
    resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//フォーマット
    resourceDesc.SampleDesc.Count = 1;//サンプル数
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;//リソースの次元
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;//深度ステンシルを許可
    //利用するheapの設定
    D3D12_HEAP_PROPERTIES heapProperties = {};
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;//VRAM上に作る
    //深度値のクリア設定
    D3D12_CLEAR_VALUE dephtClearValue{};
    dephtClearValue.DepthStencil.Depth = 1.0f;//深度値のクリア値
    dephtClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//フォーマット RESOURSEと合わせる
    //リソースの生成
    ID3D12Resource* resource = nullptr;
    HRESULT hr = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,//深度書き込み状態
        &dephtClearValue,//深度値のクリア設定
        IID_PPV_ARGS(&resource)
    );
    assert(SUCCEEDED(hr));
    return resource;


}






std::wstring wstr = L"Hello,DirectX!";
//winmain
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int){
    CoInitializeEx(0, COINIT_MULTITHREADED);

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
            ID3D12DescriptorHeap* rtvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
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
            ///ディスクプリプターレンジの作成
            D3D12_DESCRIPTOR_RANGE descriptorRange[1]{};
            descriptorRange[0].BaseShaderRegister = 0;//シェーダーのレジスタ番号0
            descriptorRange[0].NumDescriptors = 1;//ディスクリプタの数1つ
            descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
            descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//テーブルの先頭からオフセットなし
            ///
          





            // RootSignatureの作成
            D3D12_ROOT_SIGNATURE_DESC descriptionRootSignatur{};
            descriptionRootSignatur.Flags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

            ///ルートパラメータの設定
            D3D12_ROOT_PARAMETER rootParameters[3]{};
            rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
            rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//ピクセルシェーダーで使う
            rootParameters[0].Descriptor.ShaderRegister = 0;//シェーダーのレジスタ番号0とバインド
            rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
            rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;//ヴァーテックスシェーダーで使う
            rootParameters[1].Descriptor.ShaderRegister = 0;//シェーダーのレジスタ番号0とバインド
            rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//ディスクリプタテーブルを使う
            rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//ピクセルシェーダーで使う
            rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;//ディスクリプタレンジの設定
            rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);//ディスクリプタレンジの数
            descriptionRootSignatur.pParameters = rootParameters;//ルートパラメータの設定
            descriptionRootSignatur.NumParameters = _countof(rootParameters);//ルートパラメータの数

            D3D12_STATIC_SAMPLER_DESC staticSamplers[1]{};
            staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//線形フィルタリング
            staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//テクスチャのアドレスモードはラップ
            staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//テクスチャのアドレスモードはラップ
            staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//テクスチャのアドレスモードはラップ
            staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//比較関数は使用しない
            staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;//最大LODは最大値
            staticSamplers[0].ShaderRegister = 0;//シェーダーのレジスタ番号0
            staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//ピクセルシェーダーで使用する
            descriptionRootSignatur.pStaticSamplers = staticSamplers;//スタティックサンプラーの設定
            descriptionRootSignatur.NumStaticSamplers = _countof(staticSamplers);//スタティックサンプラーの数

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
            D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
            inputElementDescs[0].SemanticName = "POSITION";
            inputElementDescs[0].SemanticIndex = 0;
            inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
            inputElementDescs[1].SemanticName = "TEXCOORD";
            inputElementDescs[1].SemanticIndex = 0;
            inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
            inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
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

            ID3D12Resource* vertexResource = CreateBufferResource(device, sizeof(VartexData) * 6);
            ///
            //頂点バッファビューの設定
            D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
            //リソース先頭アドレス
            vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
            //リソースのサイズ
            vertexBufferView.SizeInBytes = sizeof(VartexData) * 6;
            vertexBufferView.StrideInBytes = sizeof(VartexData);

            //頂点データの設定
            VartexData* vertexData = nullptr;
            //書き込む為のアドレス
            vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
            //データの設定
            vertexData[0].position = { -0.5f, -0.5f, 0.0f, 1.0f };
            vertexData[0] .texcoord= { 0.0f, 1.0f };
            vertexData[1].position = { 0.0f, 0.5f, 0.0f, 1.0f };
            vertexData[1].texcoord = { 0.5f, 0.0f };
            vertexData[2].position = { 0.5f, -0.5f, 0.0f, 1.0f };
            vertexData[2].texcoord = { 1.0f, 1.0f };
            vertexData[3].position = { -0.5f, -0.5f, 0.5f, 1.0f };
            vertexData[3].texcoord = { 0.0f, 1.0f };
            vertexData[4].position = { 0.0f, 0.0f, 0.0f, 1.0f };
            vertexData[4].texcoord = { 0.5f, 0.0f };
            vertexData[5].position = { 0.5f, -0.5f, -0.5f, 1.0f };
            vertexData[5].texcoord = { 1.0f, 1.0f };
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
            *materialData =  Vector4(1.0f, 1.0f, 1.0f, 1.0f );

            ///WVP行列リソースの設定
            ID3D12Resource* wvpResource = CreateBufferResource(device, sizeof(Matrix4x4));
            //WVP行列データの設定
            Matrix4x4* wvpData = nullptr;
            //書き込む為のアドレス
            wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
            //行列の初期化
            *wvpData = Makeidetity4x4();


            //コマンドリストの初期化
            Transform transform{{1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f}};
            Transform cameraTransform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-5.0f} };
           
            ID3D12Resource* transformatiomationMatrixResource = CreateBufferResource(device, sizeof(Matrix4x4));
            //マテリアルデータの設定
            //行列の初期化
            Matrix4x4* transformatiomationMatrixDate = nullptr;
            //書き込む為のアドレス
            transformatiomationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformatiomationMatrixDate));
            //行列の初期化
           

            //  WVP行列の作成
            Matrix4x4 worldMatrix = MakeAfineMatrix(transform.scale, transform.rotate, transform.traslate);
            Matrix4x4 cameraMatrix = MakeAfineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.traslate);
            Matrix4x4 viewMatrix = Inverse(cameraMatrix);
            //透視投影行列の作成
            Matrix4x4 projectionMatirx= MakePerspectiveFovMatrix(
                0.45f, static_cast<float>(kClientWidth) / static_cast<float>(kClientHeight), 0.1f, 100.0f
            );
            //ワールド行列とビュー行列とプロジェクション行列を掛け算
            Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix,projectionMatirx));
            //行列をGPUに転送
            *transformatiomationMatrixDate = worldViewProjectionMatrix;

            //srvの設定
            ID3D12DescriptorHeap* srvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);

            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGui::StyleColorsDark();
            ImGui_ImplWin32_Init(hwnd);
            ImGui_ImplDX12_Init(
                device,
                swapChainDesc.BufferCount,
                rtvDesc.Format,
                srvDescriptorHeap,
                srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
                srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart()
            );

            ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap };

            //
            DirectX::ScratchImage mipImages = LoadTexture("resources/uvChecker.png");
            const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
            ID3D12Resource* textureResource = CresteTextureResourse(device, metadata);
            //テクスチャのアップロード
            UploadTextureData(textureResource, mipImages);

            //metaDataを基にSRVの設定
            //
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
            srvDesc.Format = metadata.format;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
            srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);//最初のミップマップ
            //SRVを作成するdescriptorの取得
            D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
            D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
            //SRVのハンドルをずらす
            textureSrvHandleCPU.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) ;
            textureSrvHandleGPU.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            //SRVの設定
            device->CreateShaderResourceView(
                textureResource,
                &srvDesc,
                textureSrvHandleCPU
            );
            ID3D12Resource* depthStencilResource = CreateDepthStencilTextureResourse(
                device,
                kClientWidth,
                kClientHeight
            );
            //DSVの設定
            ID3D12DescriptorHeap* dsvDescriptorHeap = CreateDescriptorHeap(
                device,
                D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
                1,
                false
            );
            D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
            dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//深度24ビット、ステンシル8ビット
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;//2Dテクスチャ
            //DSVのハンドルを取得
            device->CreateDepthStencilView(
                depthStencilResource,
                &dsvDesc,
                dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart()
            );







            //メインループ
    MSG msg{};
    while (msg.message != WM_QUIT) {
        //メッセージがある限りGetMessageを呼び出す
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else{
            ImGui_ImplDX12_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            ///////
            ///Update
            ///////

            transform.rotate.y += 0.03f;
            Matrix4x4 worldMatrix = MakeAfineMatrix(transform.scale,transform.rotate,transform.traslate);
            *wvpData = worldMatrix;



            ImGui::ShowDemoWindow();
          

                        ImGui::Begin("MaterialData");
            ImGui::ColorEdit4("Color", &(*materialData).x);
            ImGui::End();




            ///////
            ///Update
            ///////


            ///
            //DRAW
            ///
            ImGui::Render();
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
            commandList->SetDescriptorHeaps(1, descriptorHeaps);
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
            //マテリアルリソースの設定
            commandList->SetGraphicsRootConstantBufferView(0,materialResource->GetGPUVirtualAddress());
            //WVP行列リソースの設定
            commandList->SetGraphicsRootConstantBufferView(1,wvpResource->GetGPUVirtualAddress());
            ///
            commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);

            //
            commandList->DrawInstanced(6, 1, 0, 0);




            ///
            
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
            ///

            ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
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

    
    ///
    //ImGuiの終了処理
    ////
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();


    //リソースの解放
    CloseHandle(fenceEvent);
    fence->Release();
 //RTVの解放
    rtvDescriptorHeap->Release();

    //SRVの解放
    srvDescriptorHeap->Release();
    //テクスチャの解放
    textureResource->Release();
  
    //(*descriptorHeaps)->Release();

   
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
    vertexShaderBlob->Release();
    pixelShaderBlob->Release();
    //マテリアルリソースの解放
    materialResource->Release();
    //WVP行列リソースの解放
    wvpResource->Release();
    //トランスフォーム行列リソースの解放
    transformatiomationMatrixResource->Release();
    //テクスチャリソースの解放
    depthStencilResource->Release();






    




    
CoUninitialize();
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