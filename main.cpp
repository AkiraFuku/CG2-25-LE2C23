#include<Windows.h>
#include<cstdint>
#include<iostream>
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
#include"externals/DirectXTex/d3dx12.h"
#include<vector>
#include<numbers>
#include<sstream>
#include"Audio.h"
#include <memory>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")


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
Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(
// compilerするshaderファイルのパス
const std::wstring& filePath,
//　compilerに使用するprofile
const wchar_t* profile,
//
 const Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils,
  const Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler,
 const  Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler,
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

    Microsoft::WRL::ComPtr<IDxcResult> shaderResult = nullptr;
    hr = dxcCompiler->Compile(
        &shaderSoursBuffer,
        arguments,
        _countof(arguments),
        includeHandler.Get(),
        IID_PPV_ARGS(&shaderResult)
    );
    assert(SUCCEEDED(hr));

    //警告・エラー確認
    Microsoft::WRL::ComPtr<IDxcBlobUtf8> shaderError = nullptr;
    shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
    if (shaderError != nullptr&&shaderError->GetStringLength()!=0) {
        //エラーがあった場合
        Log(os,shaderError->GetStringPointer());
        assert(false);
    }
    //compile結果をうけとってわたす
    Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob = nullptr;
    hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
    assert(SUCCEEDED(hr));
    Log(os,ConvertString(std::format(L"Compile Succeeded, path:{},profiale:{}\n", filePath, profile)));
    //解放
    return shaderBlob;


};
Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource( const  Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes){
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
            Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
           HRESULT hr = device.Get()->CreateCommittedResource(
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
Microsoft::WRL::ComPtr <ID3D12DescriptorHeap>CreateDescriptorHeap(const  Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE heepType, UINT numDescriptors, bool shaderVisible)
{
    //ディスクリプタヒープの設定
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
    heapDesc.NumDescriptors = numDescriptors;
    heapDesc.Type = heepType;
    heapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
    HRESULT hr = device.Get()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap));
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

Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResourse( const  Microsoft::WRL::ComPtr<ID3D12Device> device , const DirectX::TexMetadata& metadata)
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
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;//デフォルトヒープ
    //リソースの生成
    Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
    HRESULT hr = device.Get()->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&resource)
    );
    assert(SUCCEEDED(hr));
    return resource;


}
[[nodiscard]] //戻り値を無視しないようにするアトリビュート
Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData( const  Microsoft::WRL::ComPtr<ID3D12Resource> textur,const DirectX::ScratchImage& mipImages, const  Microsoft::WRL::ComPtr<ID3D12Device> device,
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>  commandlist)
{
    std::vector<D3D12_SUBRESOURCE_DATA> subresources;
    DirectX::PrepareUpload(
        device.Get(),
        mipImages.GetImages(),
        mipImages.GetImageCount(),
        mipImages.GetMetadata(),
        subresources
    );
    uint64_t intermediateSize = GetRequiredIntermediateSize(
        textur.Get(),
        0,//最初のサブリソース
        UINT(subresources.size())//全てのサブリソース
    );
   Microsoft::WRL::ComPtr< ID3D12Resource> intermediateResource = CreateBufferResource(device,intermediateSize);
    UpdateSubresources(
        commandlist.Get(),
        textur.Get(),//転送先のテクスチャ
        intermediateResource.Get(),//転送元のリソース
        0,//転送元のオフセット
        0,//転送先のオフセット
        UINT(subresources.size()),//サブリソースの数
        subresources.data()//サブリソースデータ
    );
    //
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;//リソースの遷移
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;//フラグなし
    barrier.Transition.pResource = textur.Get();//遷移するリソース
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;//全てのサブリソース
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;//コピー先の状態
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;//読み取り可能な状態
    commandlist.Get()->ResourceBarrier(1, &barrier);//バリアを設定
    return intermediateResource;
    
}

Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource( const  Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width,int32_t height){
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
    D3D12_HEAP_PROPERTIES heapProperties{};
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;//デフォルトヒープ
    // 深度値のクリア設定    
    D3D12_CLEAR_VALUE depthClearValue{};
    depthClearValue.DepthStencil.Depth = 1.0f;//深度値のクリア値
    depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//フォーマット
    //リソースの生成
    Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
    HRESULT hr = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,//深度書き込み状態
        &depthClearValue,//深度値のクリア設定
        IID_PPV_ARGS(&resource)
    );
    assert(SUCCEEDED(hr));
    return resource;
}


D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle( const  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap,uint32_t descriptorSize,uint32_t index)
{
    D3D12_CPU_DESCRIPTOR_HANDLE handleCPU=descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    handleCPU.ptr+=(descriptorSize*index);
    return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle( const  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap,uint32_t descriptorSize,uint32_t index)
{
    D3D12_GPU_DESCRIPTOR_HANDLE handleGPU=descriptorHeap->GetGPUDescriptorHandleForHeapStart();
    handleGPU.ptr+=(descriptorSize*index);
    return handleGPU;
}
MaterialData LoadMaterialTemplateFile(const std::string& directryPath, const std::string& filename) {
  
    //1. 変数の宣言
        
    MaterialData materialData;
    std::string line;
    std::ifstream file(directryPath + "/" + filename);//ファイルパスを結合して開く
    //2. ファイルを開く
     assert(file.is_open());//ファイルが開けたか確認
    //3. ファイルからデータを読み込みマテリアルデータを作成
     while (std::getline(file,line)){
         std::string identifier;
         std::istringstream s(line);
         s >> identifier;//行の先頭を識別子として取得

         if (identifier=="map_Kd"){

             std::string textureFileName;
                s >> textureFileName;//テクスチャファイル名を読み込み
                //テクスチャのパスを設定
                materialData.textureFilePath = directryPath + "/" + textureFileName;
         } 
     }
    
    //4. マテリアルデータを返す
     return materialData;
}
ModelData LoadObjFile(const std::string& directryPath, const std::string& filename) 
{
    //1. 変数の宣言
    ModelData modelData;
    std::vector<Vector4> positions;//頂点座標
    std::vector<Vector3> normals;//法線ベクトル
    std::vector<Vector2> texcoords;//テクスチャ座標
    std::string line;
    //2. ファイルを開く
    std::ifstream file(directryPath + "/" + filename);//ファイルパスを結合して開く
    assert(file.is_open());//ファイルが開けたか確認

    //3. ファイルからデータを読み込みモデルデータを作成
    while (std::getline(file,line)){
        std::string identifier;
        std::istringstream s(line);
        s >> identifier;//行の先頭を識別子として取得
         ///
         if (identifier=="v"){
             Vector4 position;
             s >> position.x >> position.y >> position.z;//頂点座標を読み込み
             position.w = 1.0f; // w成分を1.0に設定
             position.x *= -1.0f; // X軸を反転
             positions.push_back(position);//頂点座標を追加
         } else if(identifier=="vt"){
             Vector2 texcoord;
             s >> texcoord.x >> texcoord.y;//テクスチャ座標を読み込み
             // OpenGLとDirectXでY軸の方向が異なるため、Y座標を反転
             texcoord.y = 1.0f - texcoord.y;
             texcoords.push_back(texcoord);//テクスチャ座標を追加
         } else if(identifier=="vn"){
             Vector3 normal;
             s >> normal.x >> normal.y >> normal.z;//法線ベクトルを読み込み
             normal.x *= -1.0f; // X軸を反転
             normals.push_back(normal);

         } else if(identifier=="f"){
            VertexData Triangle[3];
            //面は三角形限定
             for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex){
                 std::string vertexDefinition;
                 s >> vertexDefinition;//頂点定義を読み込み
                 //頂点の要素へのIndexは「位置/UV/法線」の順番で格納されているので、分解して取得
                 std::istringstream v(vertexDefinition);
                 uint32_t elementIndices[3];//位置、UV、法線のインデックス
                 for (uint32_t element = 0; element < 3; ++element){
                     std::string index;
                     std::getline(v, index, '/');//スラッシュで区切って取得
                     elementIndices[element] = std::stoi(index);//文字列を整数に変換
                 }
                 // 要素のインデックスから頂点データを構築
                 Vector4 position = positions[elementIndices[0] - 1];//1から始まるので-1
                 Vector2 texcoord = texcoords[elementIndices[1] - 1];//1から始まるので-1
                 Vector3 normal = normals[elementIndices[2] - 1];//1から始まるので-1
                 
             //    VertexData vertex = { position, texcoord, normal };//頂点データを構築
             //    modelData.vertices.push_back(vertex);//モデルデータに頂点を追加
                 Triangle[faceVertex] = { position, texcoord, normal };//頂点データを構築
             }
             modelData.vertices.push_back(Triangle[0]);
             modelData.vertices.push_back(Triangle[1]);
             modelData.vertices.push_back(Triangle[2]);
         } else if(identifier=="mtllib")//マテリアルライブラリの読み込み
         {
             std::string materialFileName;
             s >> materialFileName;//マテリアルファイル名を読み込み
             //マテリアルデータを読み込む
             modelData.material = LoadMaterialTemplateFile(directryPath, materialFileName);
         } 
      
    }
    //4. モデルデータを返す
    return modelData;
   
}

struct D3DResourceLeakChecker{
    ~D3DResourceLeakChecker(){
        // D3D12デバッグレイヤーを有効にしている場合、リソースのリークをチェック
        Microsoft::WRL::ComPtr<IDXGIDebug1> debug;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
        debug->ReportLiveObjects(DXGI_DEBUG_ALL,DXGI_DEBUG_RLO_ALL);
        debug->ReportLiveObjects(DXGI_DEBUG_APP,DXGI_DEBUG_RLO_ALL);
        debug->ReportLiveObjects(DXGI_DEBUG_D3D12,DXGI_DEBUG_RLO_ALL);

       
        }
    }
};










std::wstring wstr = L"Hello,DirectX!";
//winmain
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int){

    D3DResourceLeakChecker LeakCheck;
    //D3D12の初期化
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
            //

            //描画初期化処理
            // DXGIファクトリーの作成
            Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory=nullptr;
           // IDXGIFactory7* dxgiFactory = nullptr;
            // 
            HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
            //
            assert(SUCCEEDED(hr));

#ifdef _DEBUG

            //デバッグレイヤーの有効
            Microsoft::WRL::ComPtr<ID3D12Debug1> debugController = nullptr;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))){
                debugController->EnableDebugLayer();
                //デバッグレイヤーの詳細な情報を取得
                debugController->SetEnableGPUBasedValidation(TRUE);

            }
          
#endif // _DEBUG


            //アダプターの作成
            Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter = nullptr;
            //IDXGIAdapter4* useAdapter = nullptr;
            //良い順番のアダプターを探す
            for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i,DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                IID_PPV_ARGS(&useAdapter))!=DXGI_ERROR_NOT_FOUND ; ++i){
                ///アダプターの情報を取得
                DXGI_ADAPTER_DESC3 adapterDesc{};
                hr = useAdapter.Get()->GetDesc3(&adapterDesc);
                assert(SUCCEEDED(hr));
                if (!(adapterDesc.Flags&DXGI_ADAPTER_FLAG3_SOFTWARE)){
                    Log(logStream,ConvertString(std::format(L"Use Adapter:{}\n",adapterDesc.Description)));
                    break;
                }
                useAdapter=nullptr;
            }
            assert(useAdapter != nullptr);

            Microsoft::WRL::ComPtr<ID3D12Device>  device= nullptr;
           

            D3D_FEATURE_LEVEL featureLevels[] ={
                D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0,
            };
            const char* featureLevelStrings[] = {"12_2", "12_1", "12_0"};
            for (size_t i = 0; i < _countof(featureLevels); i++){
                hr = D3D12CreateDevice(
                    useAdapter.Get(),
                    featureLevels[i],
                    IID_PPV_ARGS(&device));

                 if (SUCCEEDED(hr)){
                    Log(logStream, (std::format("Use FeatureLevel : {}\n", featureLevelStrings[i])));
                    break;

                 }
            }
            assert(device!=nullptr);
            Log(logStream, "Complete create D3D12Device!!!\n" );
            const uint32_t descriptorSizeSRV=device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            const uint32_t descriptorSizeRTV=device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            const uint32_t descriptorSizeDSV=device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

            //ここから書く　外部入力



#ifdef _DEBUG
            Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
            if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue))))
            {
                ///深刻なエラーを出力・停止
                infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
                ///エラーを出力・停止
                infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
                ///警告を出力/停止
                infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
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
              Microsoft::WRL::ComPtr<ID3D12CommandQueue>commandQueue=nullptr;
           // ID3D12CommandQueue* commandQueue = nullptr;
            D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
            hr = device->CreateCommandQueue(
                &commandQueueDesc,
                IID_PPV_ARGS(&commandQueue));
            assert(SUCCEEDED(hr));
            //コマンドアロケーターの作成
            Microsoft::WRL::ComPtr<ID3D12CommandAllocator>commandAllocator=nullptr;
            //ID3D12CommandAllocator* commandAllocator = nullptr;
            hr = device->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                IID_PPV_ARGS(&commandAllocator));
            assert(SUCCEEDED(hr));
            //コマンドリストの作成
            Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
            //ID3D12GraphicsCommandList* commandList = nullptr;
            hr =device->CreateCommandList(
                0,
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                commandAllocator.Get(), nullptr,
                IID_PPV_ARGS(&commandList)
            );
            assert(SUCCEEDED(hr));
            //スワップチェーンの作成
            Microsoft::WRL::ComPtr<IDXGISwapChain4>swapChain=nullptr;
           // IDXGISwapChain4* swapChain = nullptr;
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
                commandQueue.Get(),
                hwnd,
                &swapChainDesc,
                nullptr,
                nullptr,
                reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf())
            );
            assert(SUCCEEDED(hr));

            //ディスクプリプターヒープの作成
            Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap =  CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);;
            //ID3D12DescriptorHeap* rtvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
            //スワップチェーンからリソースをひっぱる
            Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources[2] = { nullptr };
            //ID3D12Resource* swapChainResources[2] = {nullptr};
            hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
            assert(SUCCEEDED(hr));
            hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
            assert(SUCCEEDED(hr));
            // RTVの作成
            D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
            rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//出力結果をSRGBに変換・書き込み
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;//2Dテクスチャ
            //ディスクリプタの先頭を取得
           // D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

 
            //ディスクリプタ２つ用意
            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
            //rtv一つ目 先頭を入れる
            //rtvHandles[0] = rtvStartHandle;
            rtvHandles[0] = GetCPUDescriptorHandle(rtvDescriptorHeap,descriptorSizeRTV,0);
            device->CreateRenderTargetView(
                swapChainResources[0].Get(),
                &rtvDesc,
                rtvHandles[0]
            );
            //rtv二つ目 ハンドルを得る
           // rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            rtvHandles[1]= GetCPUDescriptorHandle(rtvDescriptorHeap,descriptorSizeRTV,1);
            //2つ目のRTVを作成
            device->CreateRenderTargetView(
                swapChainResources[1].Get(),
                &rtvDesc,
                rtvHandles[1]
            );
      //fenceのさくせい
            Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
            //ID3D12Fence* fence = nullptr;
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
            Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils = nullptr;
           // IDxcUtils* dxcUtils = nullptr;
            Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler = nullptr;
           // IDxcCompiler3* dxcCompiler = nullptr;
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
             Microsoft::WRL::ComPtr<IDxcIncludeHandler>includeHandler = nullptr;
           // IDxcIncludeHandler* includeHandler = nullptr;
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
            D3D12_ROOT_PARAMETER rootParameters[4]{};
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
            rootParameters[3].ParameterType=D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
            rootParameters[3].ShaderVisibility= D3D12_SHADER_VISIBILITY_PIXEL;//ピクセルシェーダーで使う
            rootParameters[3].Descriptor.ShaderRegister=1;

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
              Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
             Microsoft::WRL::ComPtr< ID3DBlob> errorBlob = nullptr;
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
             Microsoft::WRL::ComPtr<ID3D12RootSignature>rootSignature = nullptr;
            //ID3D12RootSignature* rootSignature = nullptr;
            hr = device->CreateRootSignature(
                0,
                signatureBlob.Get()->GetBufferPointer(),
                signatureBlob.Get()->GetBufferSize(),
                IID_PPV_ARGS(&rootSignature)
            );
            assert(SUCCEEDED(hr));
            




            //InputLayoutの設定
            D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
            inputElementDescs[0].SemanticName = "POSITION";
            inputElementDescs[0].SemanticIndex = 0;
            inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
            //
            inputElementDescs[1].SemanticName = "TEXCOORD";
            inputElementDescs[1].SemanticIndex = 0;
            inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
            inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
            //
            inputElementDescs[2].SemanticName = "NORMAL";
            inputElementDescs[2].SemanticIndex = 0;
            inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
            inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;


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
            rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;//カリングなし
                //BACK;

            rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
            //shaderのコンパイル
              Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = CompileShader(
                L"Object3D.vs.hlsl",
                L"vs_6_0",
                dxcUtils,
                dxcCompiler,
                includeHandler,
                logStream
            );
            assert(vertexShaderBlob.Get() != nullptr);

              Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = CompileShader(
                L"Object3D.ps.hlsl",
                L"ps_6_0",
                dxcUtils,
                dxcCompiler,
                includeHandler,
                logStream
            );
            assert(pixelShaderBlob.Get() != nullptr);
            ////
              Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = CreateDepthStencilTextureResource(device, kClientWidth, kClientHeight);
           
              Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
            //深度ステンシルビューの設定
            D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
            dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//深度ステンシルのフォーマット
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;//2Dテクスチャ
            device->CreateDepthStencilView(
                depthStencilResource.Get(),
                &dsvDesc,
                dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart()
            );

            //DSSの設定
            D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
            depthStencilDesc.DepthEnable = true;//深度テストを有効にする
            //書き込み
            depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
            //比較関数
            depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

            //PSOの生成
            D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicPipelineStateDesc{};
            graphicPipelineStateDesc.pRootSignature = rootSignature.Get();
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
           
            //深度ステンシルビューの設定
            graphicPipelineStateDesc.DepthStencilState = depthStencilDesc;//PSOにDSSを設定
            graphicPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;//深度ステンシルビューのフォーマット
            ////
            //PSOの生成
              Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
            hr = device->CreateGraphicsPipelineState(
                &graphicPipelineStateDesc,
                IID_PPV_ARGS(&graphicsPipelineState)
            );
            assert(SUCCEEDED(hr));

             enum UseModel
             {
                    kUseModelResourse,
                    kUseModelSphere,
                    kUseModelSuzanne,

             };
         
             UseModel useModel = kUseModelSuzanne;
                 
            ///
            
            ModelData modelData2 = LoadObjFile("resources", "Suzanne.obj");
            
            //頂点リソース

                 Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource2 =CreateBufferResource(device, sizeof(VertexData)*modelData2.vertices.size());
            //頂点バッファビューの設定
            D3D12_VERTEX_BUFFER_VIEW vertexBufferView2{};
            vertexBufferView2.BufferLocation = vertexResource2->GetGPUVirtualAddress();//リソースの先頭アドレス
            vertexBufferView2.SizeInBytes = UINT(sizeof(VertexData) * modelData2.vertices.size());//リソースのサイズ
            vertexBufferView2.StrideInBytes = sizeof(VertexData);//頂点のサイズ
            //頂点データの書き込み
            VertexData* vertexData2 = nullptr;
            vertexResource2.Get()->Map(0, nullptr, reinterpret_cast<void**>(&vertexData2));
            std::memcpy(vertexData2, modelData2.vertices.data(), sizeof(VertexData)* modelData2.vertices.size());

ModelData modelData = LoadObjFile("resources", "axis.obj");
              Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource =CreateBufferResource(device, sizeof(VertexData)*modelData.vertices.size());
            //頂点バッファビューの設定
            D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
            vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();//リソースの先頭アドレス
            vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());//リソースのサイズ
            vertexBufferView.StrideInBytes = sizeof(VertexData);//頂点のサイズ
            //頂点データの書き込み
            VertexData* vertexData = nullptr;
            vertexResource.Get()->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
            std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData)* modelData.vertices.size());




                 // 球の描画
            const uint32_t kSubdivision=32;
           Microsoft::WRL::ComPtr<ID3D12Resource>vertexResourceSphere =CreateBufferResource(device, (sizeof(VertexData) * 4)*kSubdivision*(kSubdivision+1));
            D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSphere{};
            //リソース先頭アドレス
            vertexBufferViewSphere.BufferLocation = vertexResourceSphere->GetGPUVirtualAddress();
            //リソースのサイズ
            vertexBufferViewSphere.SizeInBytes = sizeof(VertexData) * 4*kSubdivision*(kSubdivision+1);
            vertexBufferViewSphere.StrideInBytes = sizeof(VertexData);

            VertexData* vertexDataSphere=nullptr;
            vertexResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSphere));
            //インデックスバッファビューの設定
            
            Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceSphere =CreateBufferResource(device, (sizeof(uint32_t) * 6)*kSubdivision*(kSubdivision+1));
            D3D12_INDEX_BUFFER_VIEW indexBufferViewSphere{};

            indexBufferViewSphere.BufferLocation =indexResourceSphere ->GetGPUVirtualAddress();
            indexBufferViewSphere.SizeInBytes = sizeof(uint32_t) * 6 * kSubdivision * (kSubdivision + 1);
            indexBufferViewSphere.Format = DXGI_FORMAT_R32_UINT;//頂点のフォーマット
            uint32_t* indexDataSphere = nullptr;
            indexResourceSphere ->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSphere));

            const float kLonEvery =std::numbers::pi_v<float>*2.0f/static_cast<float>(kSubdivision);//経度
            const float kLatEvery =std::numbers::pi_v<float>/static_cast<float>(kSubdivision);//緯度
            for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex)
            {
                float lat =-std::numbers::pi_v<float>/2.0f+kLatEvery*latIndex;
                for (uint32_t lonIndex = 0; lonIndex <= kSubdivision; ++lonIndex)
                {
                   
                    float lon=lonIndex*kLonEvery + std::numbers::pi_v<float> / 2.0f;

                    //頂点データの設定
                    //A,B,C,Dの4つの頂点を設定

                    //左上の頂点A
                    VertexData verA={
                        {
                            std::cosf(lat)* std::cosf(lon),
                            std::sinf(lat),
                            std::cosf(lat)* std::sinf(lon),
                            1.0f
                        },
                        {
                            float(lonIndex)/float(kSubdivision),
                            1.0f-float(latIndex)/float(kSubdivision)

                        
                        },
                        { 
                            std::cosf(lat)* std::cosf(lon),
                            std::sinf(lat),
                            std::cosf(lat)* std::sinf(lon)
                        }
                    };
                    //右上の頂点B
                    VertexData verB={
                        {
                            std::cosf(lat+kLatEvery)* std::cosf(lon),
                            std::sinf(lat+kLatEvery),
                            std::cosf(lat+kLatEvery)* std::sinf(lon),
                            1.0f
                        },
                        {
                              float(lonIndex)/float(kSubdivision),
                            1.0f-float(latIndex+1)/float(kSubdivision)
                        
                        },
                        { 
                            std::cosf(lat+kLatEvery)* std::cosf(lon),
                            std::sinf(lat+kLatEvery),
                            std::cosf(lat+kLatEvery)* std::sinf(lon)
                        }
                    };

                    //左下の頂点C
                    VertexData verC={
                        {
                            std::cosf(lat)* std::cosf(lon+kLonEvery),
                            std::sinf(lat),
                            std::cosf(lat)* std::sinf(lon+kLonEvery),
                            1.0f
                        },
                        {

                          float(lonIndex+1)/float(kSubdivision),
                            1.0f-float(latIndex)/float(kSubdivision)
                        },
                        {
                            std::cosf(lat)* std::cosf(lon+kLonEvery),
                            std::sinf(lat),
                            std::cosf(lat)* std::sinf(lon+kLonEvery)
                        }

                    };
                    //右下の頂点D
                    VertexData verD={
                        {
                            std::cosf(lat+kLatEvery)*std::cosf(lon+kLonEvery),
                            std::sinf(lat+kLatEvery),
                            std::cosf(lat+kLatEvery)* std::sinf(lon+kLonEvery),
                            1.0f
                        },
                        {
                              float(lonIndex+1)/float(kSubdivision),
                            1.0f-float(latIndex+1)/float(kSubdivision)
                        
                        },
                        {
                            std::cosf(lat+kLatEvery)*std::cosf(lon+kLonEvery),
                            std::sinf(lat+kLatEvery),
                            std::cosf(lat+kLatEvery)* std::sinf(lon+kLonEvery)
                        }

                    };
                    uint32_t vertexStartIndex =(latIndex*kSubdivision+lonIndex)*4;
                    //頂点データの設定
                       
                    vertexDataSphere[vertexStartIndex+0]=verA;

                    vertexDataSphere[vertexStartIndex+1]=verB;

                    vertexDataSphere[vertexStartIndex+2]=verC;

                    vertexDataSphere[vertexStartIndex+3]=verD;

                    //インデックスデータの設定
                     uint32_t   StartIndex =(latIndex*kSubdivision+lonIndex)*6;
                     indexDataSphere[StartIndex + 0] = vertexStartIndex + 0; // A
                     indexDataSphere[StartIndex + 1] = vertexStartIndex + 1; // B
                     indexDataSphere[StartIndex + 2] = vertexStartIndex + 2; // C
                     indexDataSphere[StartIndex + 3] = vertexStartIndex + 1; // B
                     indexDataSphere[StartIndex + 4] = vertexStartIndex + 3; // D
                     indexDataSphere[StartIndex + 5] = vertexStartIndex + 2; // C



                }


            }

           




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
              Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = CreateBufferResource(device, sizeof(Material) );
            //マテリアルデータの設定
            Material* materialData = nullptr;
            //書き込む為のアドレス
            materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
            //データの設定
            materialData->color =  Vector4(1.0f, 1.0f, 1.0f, 1.0f );
            materialData->enableLighting =true;
            materialData->uvTransform = Makeidetity4x4();

            ///WVP行列リソースの設定
              Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource = CreateBufferResource(device, sizeof(TransformationMatrix));
            //WVP行列データの設定
            TransformationMatrix* wvpData = nullptr;
            //書き込む為のアドレス
            wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
            //行列の初期化
            wvpData->WVP = Makeidetity4x4();
            wvpData->World = Makeidetity4x4();


            //コマンドリストの初期化
            Transform transform{{1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f}};
            Transform cameraTransform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} };
           
              Microsoft::WRL::ComPtr<ID3D12Resource> transformatiomationMatrixResource = CreateBufferResource(device, sizeof(Matrix4x4));
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

             Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);

            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGui::StyleColorsDark();
            ImGui_ImplWin32_Init(hwnd);
            ImGui_ImplDX12_Init(
                device.Get(),
                swapChainDesc.BufferCount,
                rtvDesc.Format,
                srvDescriptorHeap.Get(),
                srvDescriptorHeap.Get()->GetCPUDescriptorHandleForHeapStart(),
                srvDescriptorHeap.Get()->GetGPUDescriptorHandleForHeapStart()
            );

            ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap.Get() };

             DirectX::ScratchImage mipImages2 = LoadTexture("resources/monsterBall.png");
            const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
            Microsoft::WRL::ComPtr<ID3D12Resource> textureResource2 = CreateTextureResourse(device, metadata2);
            //ID3D12Resource* textureResource2 = CreateTextureResourse(device, metadata2);
            //テクスチャのアップロード
              Microsoft::WRL::ComPtr<ID3D12Resource>intermediateResource2= UploadTextureData(textureResource2, mipImages2,device,commandList);
           

            //metaDataを基にSRVの設定
            //
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
            srvDesc2.Format = metadata2.format;
            srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
            srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);//最初のミップマップ
           
          D3D12_CPU_DESCRIPTOR_HANDLE  textureSrvHandleCPU2= GetCPUDescriptorHandle(srvDescriptorHeap,descriptorSizeSRV,2);
           D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2= GetGPUDescriptorHandle(srvDescriptorHeap,descriptorSizeSRV,2);
            //SRVの設定
            device->CreateShaderResourceView(
                textureResource2.Get(),
                &srvDesc2,
                textureSrvHandleCPU2
            );

            //
            DirectX::ScratchImage mipImages = LoadTexture(modelData.material.textureFilePath);
            const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
             Microsoft::WRL::ComPtr<ID3D12Resource>textureResource= CreateTextureResourse(device, metadata);
            //ID3D12Resource* textureResource = CreateTextureResourse(device, metadata);
            //テクスチャのアップロード
              Microsoft::WRL::ComPtr<ID3D12Resource>intermediateResource= UploadTextureData(textureResource, mipImages,device,commandList);
           

            //metaDataを基にSRVの設定
            //
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
            srvDesc.Format = metadata.format;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
            srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);//最初のミップマップ
           
          D3D12_CPU_DESCRIPTOR_HANDLE  textureSrvHandleCPU= GetCPUDescriptorHandle(srvDescriptorHeap,descriptorSizeSRV,1);
           D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU= GetGPUDescriptorHandle(srvDescriptorHeap,descriptorSizeSRV,1);
            //SRVの設定
            device->CreateShaderResourceView(
                textureResource.Get(),
                &srvDesc,
                textureSrvHandleCPU
            );


           //スプライトリソース
              Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourseSprite = CreateBufferResource(device, sizeof(VertexData) * 4);
            //スプライトの頂点バッファビューの設定
            D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
            //リソース先頭アドレス
            vertexBufferViewSprite.BufferLocation = vertexResourseSprite.Get()->GetGPUVirtualAddress();
            //リソースのサイズ
            vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 4;
            vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);
            
            VertexData* vertexDataSprite = nullptr;
            //書き込む為のアドレス
            vertexResourseSprite.Get()->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));
            // 左下
            vertexDataSprite[0].position = { 0.0f, 360.0f, 0.0f, 1.0f };
            vertexDataSprite[0].texcoord = { 0.0f, 1.0f };
            vertexDataSprite[0].normal = { 0.0f,0.0f, -1.0f };
            // 左上
            vertexDataSprite[1].position = { 0.0f, 0.0f, 0.0f, 1.0f };
            vertexDataSprite[1].texcoord = { 0.0f, 0.0f };
            vertexDataSprite[1].normal = { 0.0f,0.0f, -1.0f };
            // 右下
            vertexDataSprite[2].position = { 640.0f, 360.0f, 0.0f, 1.0f };
            vertexDataSprite[2].texcoord = { 1.0f, 1.0f };
            vertexDataSprite[2].normal = { 0.0f,0.0f, -1.0f };
            // 右上
            vertexDataSprite[3].position = { 640.0f, 0.0f, 0.0f, 1.0f };
            vertexDataSprite[3].texcoord = { 1.0f, 0.0f };
            vertexDataSprite[3].normal = { 0.0f,0.0f, -1.0f };
            
 ///インデックスリソース
               Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceSprite = CreateBufferResource(device, sizeof(uint32_t) * 6 );
             //インデックスバッファビューの設定
             D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
             //リソース先頭アドレス
             indexBufferViewSprite.BufferLocation = indexResourceSprite.Get()->GetGPUVirtualAddress();
             //リソースのサイズ
             indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
             indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;//32ビット整数

             uint32_t* indexDataSprite = nullptr;
             //書き込む為のアドレス
             indexResourceSprite.Get()->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
             //インデックスの設定
             indexDataSprite[0] = 0;
             indexDataSprite[1] = 1;
             indexDataSprite[2] = 2;
             indexDataSprite[3] = 2;
             indexDataSprite[4] = 1;
             indexDataSprite[5] = 3;


              Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourseSprite = CreateBufferResource(device, sizeof(TransformationMatrix));
            //スプライトの行列データの設定
            TransformationMatrix* transformationMatrixDataSprite = nullptr;
            //書き込む為のアドレス
            transformationMatrixResourseSprite.Get()->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));

            //行列の初期化
            transformationMatrixDataSprite->WVP = Makeidetity4x4();
            transformationMatrixDataSprite->World = Makeidetity4x4();

             ///マテリアルリソース
              Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSprite = CreateBufferResource(device, sizeof(Material) );
            //マテリアルデータの設定
            Material* materialDataSprite = nullptr;
            //書き込む為のアドレス
            materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
            //データの設定
            materialDataSprite->color =  Vector4(1.0f, 1.0f, 1.0f, 1.0f );
            materialDataSprite->enableLighting =false;
            materialDataSprite->uvTransform = Makeidetity4x4();
            Transform transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
            //スプライトの行列の初期化
            Matrix4x4 worldMatrixSprite = MakeAfineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.traslate);
            Matrix4x4 viewMatrixSprite = Makeidetity4x4();
            Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f,0.0f,static_cast<float>(kClientWidth),static_cast<float>(kClientHeight),0.0f,100.0f);
            //スプライトのワールド行列とビュー行列とプロジェクション行列を掛け算
            Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
            transformationMatrixDataSprite->WVP = worldViewProjectionMatrixSprite;
            transformationMatrixDataSprite->World=worldMatrixSprite;

            bool useMonstorBall =false;

            //平行光源
               Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResourse=CreateBufferResource(device,sizeof(DirectionalLight));
             DirectionalLight* directionalLightData=nullptr;
             directionalLightResourse.Get()->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
             directionalLightData->color={1.0f,1.0f,1.0f,1.0f};
             directionalLightData->direction= {0.0f,-1.0f,0.0f};
             directionalLightData->intensity=1.0f;

             //uvTransformの初期化
             Transform uvTransformSprite{
                 {1.0f,1.0f,1.0f},
                 {0.0f,0.0f,0.0f},
                 {0.0f,0.0f,0.0f}
             };

           
             
             Audio* audio = new Audio();
             audio->Initialize();
             Audio::SoundData soundData1 = Audio::SoundLoadWave("resources/fanfare.wav");
            
             audio->PlayAudio(soundData1);

            

            
          
        

          

           
            
    



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

           // transform.rotate.y += 0.03f;
          
          
            

            
           // ImGui::ShowDemoWindow();
          

            ImGui::Begin("MaterialData");
            ImGui::DragFloat3("Camera Transrate",&(cameraTransform.traslate.x));
            ImGui::DragFloat3("Camera rotateate",&(cameraTransform.rotate.x));
            ImGui::ColorEdit4("Color", &(materialData->color).x); 
            bool enableLighting = materialData->enableLighting != 0; // Convert int32_t to bool
            ImGui::Checkbox("enable", &enableLighting);
            materialData->enableLighting = enableLighting; // Update the original value after modification
            ImGui::DragFloat3("rotate",&(transform.rotate.x));
            ImGui::DragFloat3("traslate", &(transform.traslate.x));
            ImGui::Checkbox("useMonsterBall",&useMonstorBall);
            ImGui::ColorEdit4("ColorSprite", &(materialDataSprite->color).x); 
            ImGui::DragFloat3("traslateSprite",&(transformSprite.traslate.x));
            ImGui::ColorEdit4("LightColor", &(directionalLightData->color).x); 
            ImGui::DragFloat3("Light Direction", &(directionalLightData->direction.x));
            ImGui::InputFloat("intensity",&(directionalLightData->intensity));

            ImGui::DragFloat2("uvTransformSprite", &uvTransformSprite.traslate.x,0.01f,-10.0f,10.0f);
            ImGui::DragFloat2("uvScaleSprite", &uvTransformSprite.scale.x,0.01f,-10.0f,10.0f);
            ImGui::SliderAngle("uvRotateSprite", &uvTransformSprite.rotate.z);
            ImGui::End();
            Matrix4x4 cameraMatrix = MakeAfineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.traslate);
            Matrix4x4 viewMatrix = Inverse(cameraMatrix);
            Matrix4x4 worldMatrix = MakeAfineMatrix(transform.scale,transform.rotate,transform.traslate);
            wvpData->WVP = Multiply(worldMatrix, Multiply(viewMatrix,projectionMatirx));
            wvpData->World=worldMatrix;
            Matrix4x4 worldMatrixSprite = MakeAfineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.traslate);
            Matrix4x4 viewMatrixSprite = Makeidetity4x4();
            Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f,0.0f,static_cast<float>(kClientWidth),static_cast<float>(kClientHeight),0.0f,100.0f);
            transformationMatrixDataSprite->WVP =  Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));;
            transformationMatrixDataSprite->World=worldMatrixSprite;
             directionalLightData->direction= Normalize(directionalLightData->direction);

             Matrix4x4 uvTransformMatrix=MakeScaleMatrix(uvTransformSprite.scale);
             uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
             uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.traslate));
             materialDataSprite->uvTransform = uvTransformMatrix;


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
            barrier.Transition.pResource = swapChainResources[backBufferIndex].Get();
            //遷移前（現在）のリソース状態
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
            //遷移後のリソース状態
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
            //transitionバリアーを張る
            commandList->ResourceBarrier(1, &barrier);
            ///
          //  D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
            D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = GetCPUDescriptorHandle(dsvDescriptorHeap,descriptorSizeDSV,0);
            commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], FALSE, &dsvHandle);
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
            //深度ステンシルビューのクリア
            commandList->ClearDepthStencilView(
                dsvHandle,
                D3D12_CLEAR_FLAG_DEPTH ,
                1.0f, 0, 0, nullptr
            );
            

            ///
            commandList->SetDescriptorHeaps(1, descriptorHeaps);
            ///


            commandList->RSSetViewports(1, &viewport);//ビューポートの設定
            commandList->RSSetScissorRects(1, &scissorRect);//シザー矩形の設定
            // RootSignatureの設定
            commandList->SetGraphicsRootSignature(rootSignature.Get());
            //PSOの設定
            commandList->SetPipelineState(graphicsPipelineState.Get());
            //VBVの設定
           
            //IBVの設定
           // commandList->IASetIndexBuffer(&indexBufferView);
            //形状の設定
            commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            //マテリアルリソースの設定
            commandList->SetGraphicsRootConstantBufferView(0,materialResource.Get()->GetGPUVirtualAddress());
            //WVP行列リソースの設定
            commandList->SetGraphicsRootConstantBufferView(1,wvpResource.Get()->GetGPUVirtualAddress());
            ///
            commandList->SetGraphicsRootDescriptorTable(2,useMonstorBall?textureSrvHandleGPU2:textureSrvHandleGPU);
            // 追加: 平行光源CBVをバインド
            commandList->SetGraphicsRootConstantBufferView(3, directionalLightResourse.Get()->GetGPUVirtualAddress());
            //
            //描画コマンド
            switch (useModel)
            {

            case kUseModelResourse:
                 commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
                  commandList->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);
                break;
            case kUseModelSuzanne:
               commandList->IASetVertexBuffers(0, 1, &vertexBufferView2);
                  commandList->DrawInstanced(UINT(modelData2.vertices.size()), 1, 0, 0);
                break;
            case kUseModelSphere:
                 //VBVの設定
            commandList->IASetVertexBuffers(0, 1, &vertexBufferViewSphere);
            //IBVの設定
            commandList->IASetIndexBuffer(&indexBufferViewSphere);
                commandList->DrawIndexedInstanced(6*kSubdivision*kSubdivision, 1, 0, 0,0);
                break;
            default:
                break;
            }
            
            
            ///
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
           
           
            ///スプライトの描画
            commandList->SetGraphicsRootConstantBufferView(0,materialResourceSprite.Get()->GetGPUVirtualAddress());
            commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
            commandList->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);
            commandList->IASetIndexBuffer(&indexBufferViewSprite);
            commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResourseSprite->GetGPUVirtualAddress());
            //commandList->DrawInstanced(6, 1, 0, 0);
            commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
            

            

            ///

            ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());
            //transitionバリアーを張る
            commandList->ResourceBarrier(1, &barrier);
            ///
            //コマンドリストをクローズ
            hr = commandList->Close();
            assert(SUCCEEDED(hr));
            //GPUにコマンドリストを実行
            ID3D12CommandList* commandLists[] = { commandList.Get() };
            commandQueue->ExecuteCommandLists(1, commandLists);
            //スワップチェーンをフリップ
            swapChain->Present(1, 0);
            ///gpuの完了を待つ
            fenceValue++;
            commandQueue->Signal(fence.Get(), fenceValue);
            ///
            if (fence.Get()->GetCompletedValue()<fenceValue)
            {

                fence.Get()->SetEventOnCompletion(fenceValue, fenceEvent);
                WaitForSingleObject(fenceEvent, INFINITE);
            }
            //コマンドアロケーターをリセット
            hr = commandAllocator->Reset();
            assert(SUCCEEDED(hr));
            //コマンドリストをリセット
            hr = commandList->Reset(commandAllocator.Get(), nullptr);
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
 
    delete audio;





    




    
CoUninitialize();
    //デバッグレイヤーの解放

    CloseWindow(hwnd);

    ///デバッグレイヤーのライブオブジェクトのレポート

  

    
	return 0;
}