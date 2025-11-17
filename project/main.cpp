#include<Windows.h>
#include<cstdint>
#include<iostream>
#include<string>
#include<filesystem>
#include<fstream>
#include<chrono>
#include<strsafe.h>
#include<dbghelp.h>
#pragma comment(lib,"Dbghelp.lib")
#include<dxgidebug.h>
#pragma comment(lib,"dxguid.lib")
#include<dxcapi.h>
#include "engine/base/WinApp.h"
#include "engine/base/DXCommon.h"
#pragma comment(lib,"dxcompiler.lib")
#include"engine/math/MassFunction.h"
#include"externals/imgui/imgui.h"
#include"externals/imgui/imgui_impl_dx12.h"
#include"externals/imgui/imgui_impl_win32.h"
#include"externals/DirectXTex/DirectXTex.h"
#include"externals/DirectXTex/d3dx12.h"

#include"engine/audio/Audio.h"
#include "engine/input/Input.h"
#include"engine/base/D3DResourceLeakChecker.h"
#include"StringUtility.h"
#include"Logger.h"

#include "engine/2d/Sprite.h"
#include "engine/2d/SpriteCommon.h"


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);





LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
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

MaterialData LoadMaterialTemplateFile(const std::string& directryPath, const std::string& filename) {

    //1. 変数の宣言

    MaterialData materialData;
    std::string line;
    std::ifstream file(directryPath + "/" + filename);//ファイルパスを結合して開く
    //2. ファイルを開く
    assert(file.is_open());//ファイルが開けたか確認
    //3. ファイルからデータを読み込みマテリアルデータを作成
    while (std::getline(file, line)) {
        std::string identifier;
        std::istringstream s(line);
        s >> identifier;//行の先頭を識別子として取得

        if (identifier == "map_Kd") {

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
    while (std::getline(file, line)) {
        std::string identifier;
        std::istringstream s(line);
        s >> identifier;//行の先頭を識別子として取得
        ///
        if (identifier == "v") {
            Vector4 position;
            s >> position.x >> position.y >> position.z;//頂点座標を読み込み
            position.w = 1.0f; // w成分を1.0に設定
            position.x *= -1.0f; // X軸を反転
            positions.push_back(position);//頂点座標を追加
        } else if (identifier == "vt") {
            Vector2 texcoord;
            s >> texcoord.x >> texcoord.y;//テクスチャ座標を読み込み
            // OpenGLとDirectXでY軸の方向が異なるため、Y座標を反転
            texcoord.y = 1.0f - texcoord.y;
            texcoords.push_back(texcoord);//テクスチャ座標を追加
        } else if (identifier == "vn") {
            Vector3 normal;
            s >> normal.x >> normal.y >> normal.z;//法線ベクトルを読み込み
            normal.x *= -1.0f; // X軸を反転
            normals.push_back(normal);

        } else if (identifier == "f") {
            VertexData Triangle[3];
            //面は三角形限定
            for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
                std::string vertexDefinition;
                s >> vertexDefinition;//頂点定義を読み込み
                //頂点の要素へのIndexは「位置/UV/法線」の順番で格納されているので、分解して取得
                std::istringstream v(vertexDefinition);
                uint32_t elementIndices[3];//位置、UV、法線のインデックス
                for (uint32_t element = 0; element < 3; ++element) {
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
            modelData.vertices.push_back(Triangle[2]);
            modelData.vertices.push_back(Triangle[1]);
            modelData.vertices.push_back(Triangle[0]);
        } else if (identifier == "mtllib")//マテリアルライブラリの読み込み
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













std::wstring wstr = L"Hello,DirectX!";
//winmain
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

    D3DResourceLeakChecker LeakCheck;
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

    WinApp* winApp = nullptr;

    winApp = new WinApp();
    winApp->Initialize();

    DXCommon* dxCommon = nullptr;
    dxCommon = new DXCommon();
    dxCommon->Initialize(winApp);


    Logger::Log(StringUtility::ConvertString(std::format(L"WSTRING{}\n", wstr)));
    HRESULT hr;



    //ここから書く　外部入力
    Input* input = nullptr;
    input = new Input();
    input->Initialize(winApp);


    SpriteCommon* spritecommon;
    spritecommon->Initialize();


    



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
    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//ピクセルシェーダーで使う
    rootParameters[3].Descriptor.ShaderRegister = 1;

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
    if (FAILED(hr)) {
        Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }
    //バイナリを元にルートシグネチャー生成
    Microsoft::WRL::ComPtr<ID3D12RootSignature>rootSignature = nullptr;
    //ID3D12RootSignature* rootSignature = nullptr;
    hr = dxCommon->GetDevice()->CreateRootSignature(
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
    blendDesc.RenderTarget[0].RenderTargetWriteMask =
        D3D12_COLOR_WRITE_ENABLE_ALL;
    //RasteriwrStateの設定
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;//カリングなし
    //BACK;

    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    //shaderのコンパイル
    Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = dxCommon->CompileShader(
        L"resources/shaders/Object3d/Object3D.vs.hlsl",
        L"vs_6_0"


    );
    assert(vertexShaderBlob.Get() != nullptr);

    Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = dxCommon->CompileShader(
        L"resources/shaders/Object3d/Object3D.ps.hlsl",
        L"ps_6_0"

    );
    assert(pixelShaderBlob.Get() != nullptr);
   

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
    graphicPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
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
    hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(
        &graphicPipelineStateDesc,
        IID_PPV_ARGS(&graphicsPipelineState)
    );
    assert(SUCCEEDED(hr));
    ///
    ModelData modelData = LoadObjFile("resources", "axis.obj");

    //頂点リソース
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = dxCommon->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());
    //頂点バッファビューの設定
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
    vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();//リソースの先頭アドレス
    vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());//リソースのサイズ
    vertexBufferView.StrideInBytes = sizeof(VertexData);//頂点のサイズ
    //頂点データの書き込み
    VertexData* vertexData = nullptr;
    vertexResource.Get()->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
    std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());






   

    ///マテリアルリソース
    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = dxCommon->CreateBufferResource(sizeof(Material));
    //マテリアルデータの設定
    Material* materialData = nullptr;
    //書き込む為のアドレス
    materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
    //データの設定
    materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    materialData->enableLighting = true;
    materialData->uvTransform = Makeidetity4x4();

    ///WVP行列リソースの設定
    Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
    //WVP行列データの設定
    TransformationMatrix* wvpData = nullptr;
    //書き込む為のアドレス
    wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
    //行列の初期化
    wvpData->WVP = Makeidetity4x4();
    wvpData->World = Makeidetity4x4();


    //コマンドリストの初期化
    Transform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
    Transform cameraTransform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} };

    Microsoft::WRL::ComPtr<ID3D12Resource> transformatiomationMatrixResource = dxCommon->CreateBufferResource(sizeof(Matrix4x4));
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
    Matrix4x4 projectionMatirx = MakePerspectiveFovMatrix(
        0.45f, static_cast<float>(WinApp::kClientWidth) / static_cast<float>(WinApp::kClientHeight), 0.1f, 100.0f
    );
    //ワールド行列とビュー行列とプロジェクション行列を掛け算
    Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatirx));
    //行列をGPUに転送
    *transformatiomationMatrixDate = worldViewProjectionMatrix;

    
    DirectX::ScratchImage mipImages2 = dxCommon->LoadTexture(modelData.material.textureFilePath);
    const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
    Microsoft::WRL::ComPtr<ID3D12Resource> textureResource2 = dxCommon->CreateTextureResourse(metadata2);
    //ID3D12Resource* textureResource2 = CreateTextureResourse(device, metadata2);
    //テクスチャのアップロード
    Microsoft::WRL::ComPtr<ID3D12Resource>intermediateResource2 = dxCommon->UploadTextureData(textureResource2, mipImages2);


    //metaDataを基にSRVの設定
    //
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
    srvDesc2.Format = metadata2.format;
    srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
    srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);//最初のミップマップ

    D3D12_CPU_DESCRIPTOR_HANDLE  textureSrvHandleCPU2 = dxCommon->GetSRVCPUDescriptorHandle(2);
    D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = dxCommon->GetSRVGPUDescriptorHandle(2);
    //SRVの設定
    dxCommon->GetDevice()->CreateShaderResourceView(
        textureResource2.Get(),
        &srvDesc2,
        textureSrvHandleCPU2
    );

    //
    DirectX::ScratchImage mipImages = dxCommon->LoadTexture("resources/uvChecker.png");
    const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
    Microsoft::WRL::ComPtr<ID3D12Resource>textureResource = dxCommon->CreateTextureResourse(metadata);
    //ID3D12Resource* textureResource = CreateTextureResourse(device, metadata);
    //テクスチャのアップロード
    Microsoft::WRL::ComPtr<ID3D12Resource>intermediateResource = dxCommon->UploadTextureData(textureResource, mipImages);


    //metaDataを基にSRVの設定
    //
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = metadata.format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
    srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);//最初のミップマップ

    D3D12_CPU_DESCRIPTOR_HANDLE  textureSrvHandleCPU = dxCommon->GetSRVCPUDescriptorHandle(1);
    D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = dxCommon->GetSRVGPUDescriptorHandle(1);
    //SRVの設定
    dxCommon->GetDevice()->CreateShaderResourceView(
        textureResource.Get(),
        &srvDesc,
        textureSrvHandleCPU
    );


    //スプライトリソース
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourseSprite = dxCommon->CreateBufferResource(sizeof(VertexData) * 4);
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
    Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceSprite = dxCommon->CreateBufferResource(sizeof(uint32_t) * 6);
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


    Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourseSprite = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
    //スプライトの行列データの設定
    TransformationMatrix* transformationMatrixDataSprite = nullptr;
    //書き込む為のアドレス
    transformationMatrixResourseSprite.Get()->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));

    //行列の初期化
    transformationMatrixDataSprite->WVP = Makeidetity4x4();
    transformationMatrixDataSprite->World = Makeidetity4x4();

    ///マテリアルリソース
    Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSprite = dxCommon->CreateBufferResource(sizeof(Material));
    //マテリアルデータの設定
    Material* materialDataSprite = nullptr;
    //書き込む為のアドレス
    materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
    //データの設定
    materialDataSprite->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    materialDataSprite->enableLighting = false;
    materialDataSprite->uvTransform = Makeidetity4x4();
    Transform transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
    //スプライトの行列の初期化
    Matrix4x4 worldMatrixSprite = MakeAfineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.traslate);
    Matrix4x4 viewMatrixSprite = Makeidetity4x4();
    Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, static_cast<float>(WinApp::kClientWidth), static_cast<float>(WinApp::kClientHeight), 0.0f, 100.0f);
    //スプライトのワールド行列とビュー行列とプロジェクション行列を掛け算
    Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
    transformationMatrixDataSprite->WVP = worldViewProjectionMatrixSprite;
    transformationMatrixDataSprite->World = worldMatrixSprite;

    bool useMonstorBall = false;

    //平行光源
    Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResourse = dxCommon->CreateBufferResource(sizeof(DirectionalLight));
    DirectionalLight* directionalLightData = nullptr;
    directionalLightResourse.Get()->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
    directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
    directionalLightData->direction = { 0.0f,-1.0f,0.0f };
    directionalLightData->intensity = 1.0f;

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








    Sprite* sprite = new Sprite();
    sprite->Initialize();









    //メインループ

    while (true) {


        //メッセージがある限りGetMessageを呼び出す
        if (winApp->ProcessMessage()) {
            break;

        }

        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        input->Update();



        // キー入力判定
        if (input->TriggerKeyDown(DIK_D)) {
            OutputDebugStringA("DIK_D\n");
            transform.rotate.y += 0.1f; // 右に移動
        }
        if (input->TriggerKeyDown(DIK_A))
        {
            OutputDebugStringA("DIK_A\n");
            transform.rotate.y -= 0.1f; // 左に移動
        }

        ///////
        ///Update
        ///////

       // 





       


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
        Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f,0.0f,static_cast<float>(WinApp::kClientWidth),static_cast<float>(WinApp::kClientHeight),0.0f,100.0f);
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
        dxCommon->PreDraw();
      
      //    // RootSignatureの設定
      //  dxCommon->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
      //  //PSOの設定
      //  dxCommon->GetCommandList()->SetPipelineState(graphicsPipelineState.Get());
      //  //VBVの設定
      //  dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);


      //  //形状の設定
      //  dxCommon->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

      //  ///モデルの描画
      //  //マテリアルリソースの設定
      //  dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource.Get()->GetGPUVirtualAddress());
      //  //WVP行列リソースの設定
      //  dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource.Get()->GetGPUVirtualAddress());
      //  ///
      //  dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, useMonstorBall ? textureSrvHandleGPU2 : textureSrvHandleGPU);
      //  // 追加: 平行光源CBVをバインド
      //  dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResourse.Get()->GetGPUVirtualAddress());
      //  //
      //  //描画コマンド

      ////   commandList->DrawIndexedInstanced(6*kSubdivision*kSubdivision, 1, 0, 0,0);
      //  dxCommon->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);
        ///
     //   barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
     //   barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;


        /////スプライトの描画
        //dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0,materialResourceSprite.Get()->GetGPUVirtualAddress());
        //dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
        //dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);
        //dxCommon->GetCommandList()->IASetIndexBuffer(&indexBufferViewSprite);
        //dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResourseSprite->GetGPUVirtualAddress());
        ////commandList->DrawInstanced(6, 1, 0, 0);
        //dxCommon->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);




        ///

        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList().Get());

        dxCommon->PostDraw();
       
    }



    dxCommon->Finalize();

    delete audio;
    delete input;

    delete sprite;
    delete spritecommon;
    delete dxCommon;
    dxCommon = nullptr;


    winApp->Finalize();
    delete winApp;
    winApp = nullptr;


    return 0;
}