#include "DXCommon.h"

#include <cassert>
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#include "Logger.h"
#include "StringUtility.h"
#include <format>




void DXCommon::Initialize(WinApp* winApp)
{
    winApp_ = winApp;
    CreateDevice();
    CreateCommand();
    CreateSwapChain();
    CreateDepthStencilTextureResource();





}

void DXCommon::CreateDevice()
{
    HRESULT hr;
#ifdef _DEBUG

    //デバッグレイヤーの有効
    Microsoft::WRL::ComPtr<ID3D12Debug1> debugController = nullptr;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        debugController->EnableDebugLayer();
        //デバッグレイヤーの詳細な情報を取得
        debugController->SetEnableGPUBasedValidation(TRUE);

    }

#endif // _DEBUG
    // 
    dxgiFactory_ = nullptr;

    hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));
    //
    assert(SUCCEEDED(hr));
    //アダプターの作成
    Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter = nullptr;
    //IDXGIAdapter4* useAdapter = nullptr;
    //良い順番のアダプターを探す
    for (UINT i = 0; dxgiFactory_->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
        IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND; ++i) {
        ///アダプターの情報を取得
        DXGI_ADAPTER_DESC3 adapterDesc{};
        hr = useAdapter.Get()->GetDesc3(&adapterDesc);
        assert(SUCCEEDED(hr));
        if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
            Logger::Log(StringUtility::ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
            break;
        }
        useAdapter = nullptr;
    }
    assert(useAdapter != nullptr);

    device_ = nullptr;

    //対応するFeatureLevelでデバイスを作成
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0,
    };
    //
    const char* featureLevelStrings[] = { "12_2", "12_1", "12_0" };
    for (size_t i = 0; i < _countof(featureLevels); i++) {
        hr = D3D12CreateDevice(
            useAdapter.Get(),
            featureLevels[i],
            IID_PPV_ARGS(&device_));

        if (SUCCEEDED(hr)) {
            Logger::Log((std::format("Use FeatureLevel : {}\n", featureLevelStrings[i])));
            break;

        }
    }
    //
    assert(device_ != nullptr);

#ifdef _DEBUG
    Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
    if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&infoQueue))))
    {
        ///深刻なエラーを出力・停止
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        ///エラーを出力・停止
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
        ///警告を出力/停止
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
        //メッセージID
        D3D12_MESSAGE_ID denyIds[] = {
            D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE,

        };
        //
        D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
        D3D12_INFO_QUEUE_FILTER filter{};
        filter.DenyList.NumIDs = _countof(denyIds);
        filter.DenyList.pIDList = denyIds;
        filter.DenyList.NumSeverities = _countof(severities);
        filter.DenyList.pSeverityList = severities;
        //
        infoQueue->PushStorageFilter(&filter);

    }


#endif // _DEBUG

}

void DXCommon::CreateCommand()
{
    HRESULT hr;
    //コマンドキューの作成
    commandQueue_ = nullptr;
    // ID3D12CommandQueue* commandQueue = nullptr;
    D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
    hr = device_->CreateCommandQueue(
        &commandQueueDesc,
        IID_PPV_ARGS(&commandQueue_));
    assert(SUCCEEDED(hr));
    //コマンドアロケーターの作成
    commandAllocator_ = nullptr;
    //ID3D12CommandAllocator* commandAllocator = nullptr;
    hr = device_->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(&commandAllocator_));
    assert(SUCCEEDED(hr));
    //コマンドリストの作成
    commandList_ = nullptr;
    //ID3D12GraphicsCommandList* commandList = nullptr;
    hr = device_->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        commandAllocator_.Get(), nullptr,
        IID_PPV_ARGS(&commandList_)
    );
    assert(SUCCEEDED(hr));
}

void DXCommon::CreateSwapChain()
{
    HRESULT hr;
    //スワップチェーンの作成
    swapChain_ = nullptr;
    // IDXGISwapChain4* swapChain = nullptr;
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
    swapChainDesc.Width = WinApp::kClientWidth;//画像の幅
    swapChainDesc.Height = WinApp::kClientHeight;//画像の高さ
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//色の形式
    swapChainDesc.SampleDesc.Count = 1;//マルチサンプルしない
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//レンダリングターゲットとして使用
    swapChainDesc.BufferCount = 2;//バッファの数
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;//写したら破棄
    // コマンドキュー,ウィンドウハンドル、設定して生成
    hr = dxgiFactory_->CreateSwapChainForHwnd(
        commandQueue_.Get(),
        winApp_->GetHwnd(),
        &swapChainDesc,
        nullptr,
        nullptr,
        reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf())
    );
    assert(SUCCEEDED(hr));
}



void DXCommon::CreateDepthStencilTextureResource() {
    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Width = winApp_->kClientWidth;//幅
    resourceDesc.Height = winApp_->kClientHeight;//高さ
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
  depthStencilResource_ = nullptr;
    HRESULT hr = device_->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,//深度書き込み状態
        &depthClearValue,//深度値のクリア設定
        IID_PPV_ARGS(&depthStencilResource_)
    );
    assert(SUCCEEDED(hr));
}

void DXCommon::CreateDescriptorHeaps()
{
      descriptorSizeSRV_=device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
      descriptorSizeRTV_=device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
      descriptorSizeDSV_=device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

      //SRVヒープの作成
      srvHeap_ = CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
      //RTVヒープの作成
      rtvHeap_ = CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
      //DSVヒープの作成
      dsvHeap_ = CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);




}

Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> DXCommon::CreateDescriptorHeap(const  Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE heepType, UINT numDescriptors, bool shaderVisible)
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


