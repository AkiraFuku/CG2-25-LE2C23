#include "DXCommon.h"

#include <cassert>
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#include "Logger.h"
#include "StringUtility.h"
#include <format>
#include"externals/imgui/imgui.h"
#include"externals/imgui/imgui_impl_dx12.h"
#include"externals/imgui/imgui_impl_win32.h"





void DXCommon::Initialize(WinApp* winApp)
{
    winApp_ = winApp;
    CreateDevice();
    CreateCommand();
    CreateSwapChain();
    CreateDepthStencilTextureResource();
    CreateDescriptorHeaps();
    CreateRenderTargetView();
    CreateDepthStencilView();






}

D3D12_CPU_DESCRIPTOR_HANDLE DXCommon::GetSRVCPUDescriptorHandle(uint32_t index)
{
    return GetCPUDescriptorHandle(srvHeap_, descriptorSizeSRV_, index);
}

D3D12_GPU_DESCRIPTOR_HANDLE DXCommon::GetSRVGPUDescriptorHandle(uint32_t index)
{
    return GetGPUDescriptorHandle(srvHeap_, descriptorSizeSRV_, index);
}

void DXCommon::CreateDevice()
{

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

    hr_ = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));
    //
    assert(SUCCEEDED(hr_));
    //アダプターの作成
    Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter = nullptr;
    //IDXGIAdapter4* useAdapter = nullptr;
    //良い順番のアダプターを探す
    for (UINT i = 0; dxgiFactory_->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
        IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND; ++i) {
        ///アダプターの情報を取得
        DXGI_ADAPTER_DESC3 adapterDesc{};
        hr_ = useAdapter.Get()->GetDesc3(&adapterDesc);
        assert(SUCCEEDED(hr_));
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
        hr_ = D3D12CreateDevice(
            useAdapter.Get(),
            featureLevels[i],
            IID_PPV_ARGS(&device_));

        if (SUCCEEDED(hr_)) {
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

    //コマンドキューの作成
    commandQueue_ = nullptr;
    // ID3D12CommandQueue* commandQueue = nullptr;
    D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
    hr_ = device_->CreateCommandQueue(
        &commandQueueDesc,
        IID_PPV_ARGS(&commandQueue_));
    assert(SUCCEEDED(hr_));
    //コマンドアロケーターの作成
    commandAllocator_ = nullptr;
    //ID3D12CommandAllocator* commandAllocator = nullptr;
    hr_ = device_->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(&commandAllocator_));
    assert(SUCCEEDED(hr_));
    //コマンドリストの作成
    commandList_ = nullptr;
    //ID3D12GraphicsCommandList* commandList = nullptr;
    hr_ = device_->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        commandAllocator_.Get(), nullptr,
        IID_PPV_ARGS(&commandList_)
    );
    assert(SUCCEEDED(hr_));
}

void DXCommon::CreateSwapChain()
{

    //スワップチェーンの作成
    swapChain_ = nullptr;
    // IDXGISwapChain4* swapChain = nullptr;
  
    swapChainDesc_.Width = WinApp::kClientWidth;//画像の幅
    swapChainDesc_.Height = WinApp::kClientHeight;//画像の高さ
    swapChainDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//色の形式
    swapChainDesc_.SampleDesc.Count = 1;//マルチサンプルしない
    swapChainDesc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//レンダリングターゲットとして使用
    swapChainDesc_.BufferCount = 2;//バッファの数
    swapChainDesc_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;//写したら破棄
    // コマンドキュー,ウィンドウハンドル、設定して生成
    hr_ = dxgiFactory_->CreateSwapChainForHwnd(
        commandQueue_.Get(),
        winApp_->GetHwnd(),
        &swapChainDesc_,
        nullptr,
        nullptr,
        reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf())
    );
    assert(SUCCEEDED(hr_));
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
    descriptorSizeSRV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    descriptorSizeRTV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    descriptorSizeDSV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

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

void DXCommon::CreateRenderTargetView()
{
    //スワップチェーンからリソースをひっぱる
    hr_ = swapChain_->GetBuffer(0, IID_PPV_ARGS(&swapChainResources_[0]));
    assert(SUCCEEDED(hr_));
    hr_ = swapChain_->GetBuffer(1, IID_PPV_ARGS(&swapChainResources_[1]));
    assert(SUCCEEDED(hr_));
    // RTVの作成
  
    rtvDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//出力結果をSRGBに変換・書き込み
    rtvDesc_.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;//2Dテクスチャ

    //ディスクリプタヒープのハンドルを取得
    for (uint32_t i = 0; i < 2; i++)
    {

        rtvHandles_[i] = GetCPUDescriptorHandle(rtvHeap_, descriptorSizeRTV_, i);
        //レンダーターゲットビューの生成
        device_->CreateRenderTargetView(
            swapChainResources_[i].Get(),
            &rtvDesc_,
            rtvHandles_[i]
        );
    }
}
D3D12_CPU_DESCRIPTOR_HANDLE DXCommon::GetCPUDescriptorHandle(const  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index)
{
    D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    handleCPU.ptr += (descriptorSize * index);
    return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE DXCommon::GetGPUDescriptorHandle(const  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index)
{
    D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
    handleGPU.ptr += (descriptorSize * index);
    return handleGPU;
}

void DXCommon::CreateDepthStencilView()
{
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//深度ステンシルのフォーマット
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;//2Dテクスチャ
    device_->CreateDepthStencilView(
        depthStencilResource_.Get(),
        &dsvDesc,
        dsvHeap_->GetCPUDescriptorHandleForHeapStart()
    );
}

void DXCommon::CreateFence()
{

    //ID3D12Fence* fence = nullptr;
    uint64_t fenceValue = 0;
    hr_ = device_->CreateFence(
        fenceValue,
        D3D12_FENCE_FLAG_NONE,
        IID_PPV_ARGS(&fence_)
    );
    assert(SUCCEEDED(hr_));
    HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(fenceEvent != nullptr);
}

void DXCommon::CreateViewport()
{
    viewport_.Width = static_cast<float>(WinApp::kClientWidth);
    viewport_.Height = static_cast<float>(WinApp::kClientHeight);
    viewport_.TopLeftX = 0.0f;
    viewport_.TopLeftY = 0.0f;
    viewport_.MinDepth = 0.0f;
    viewport_.MaxDepth = 1.0f;
}

void DXCommon::CreateScissorRect()
{
    scissorRect_.left = 0;
    scissorRect_.right = WinApp::kClientWidth;
    scissorRect_.top = 0;
    scissorRect_.bottom = WinApp::kClientHeight;
}

void DXCommon::CreateDXCompiler()
{
    hr_ = DxcCreateInstance(
        CLSID_DxcUtils,
        IID_PPV_ARGS(&dxcUtils)
    );
    assert(SUCCEEDED(hr_));
    hr_ = DxcCreateInstance(
        CLSID_DxcCompiler,
        IID_PPV_ARGS(&dxcCompiler)
    );
    assert(SUCCEEDED(hr_));
    hr_ = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
    assert(SUCCEEDED(hr_));

}

void DXCommon::InitializeImGui()
{
      IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGui::GetIO().IniFilename = "externals/imgui/my_imgui_settings.ini";
            ImGui::StyleColorsDark();
            ImGui_ImplWin32_Init(winApp_->GetHwnd());
            ImGui_ImplDX12_Init(
                device_.Get(),
                swapChainDesc_.BufferCount,
                rtvDesc_.Format,
                srvHeap_.Get(),
                srvHeap_.Get()->GetCPUDescriptorHandleForHeapStart(),
                srvHeap_.Get()->GetGPUDescriptorHandleForHeapStart()
            );
}


