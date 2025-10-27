#include "DXCommon.h"

#include <cassert>
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#include "Logger.h"
#include "StringUtility.h"
#include <format>




void DXCommon::Initialize()
{



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
            for (UINT i = 0; dxgiFactory_->EnumAdapterByGpuPreference(i,DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                IID_PPV_ARGS(&useAdapter))!=DXGI_ERROR_NOT_FOUND ; ++i){
                ///アダプターの情報を取得
                DXGI_ADAPTER_DESC3 adapterDesc{};
                hr = useAdapter.Get()->GetDesc3(&adapterDesc);
                assert(SUCCEEDED(hr));
                if (!(adapterDesc.Flags&DXGI_ADAPTER_FLAG3_SOFTWARE)){
                   Logger::Log(StringUtility::ConvertString(std::format(L"Use Adapter:{}\n",adapterDesc.Description)));
                    break;
                }
                useAdapter=nullptr;
            }
            assert(useAdapter != nullptr);

            device_ = nullptr;

              //対応するFeatureLevelでデバイスを作成
            D3D_FEATURE_LEVEL featureLevels[] ={
                D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0,
            };
            //
            const char* featureLevelStrings[] = {"12_2", "12_1", "12_0"};
            for (size_t i = 0; i < _countof(featureLevels); i++){
                hr = D3D12CreateDevice(
                    useAdapter.Get(),
                    featureLevels[i],
                    IID_PPV_ARGS(&device_));

                 if (SUCCEEDED(hr)){
                   Logger::Log( (std::format("Use FeatureLevel : {}\n", featureLevelStrings[i])));
                    break;

                 }
            }
            //
            assert(device_!=nullptr);
           
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

}

void DXCommon::CreateCommand()
{
      HRESULT hr;
     //コマンドキューの作成
            commandQueue_=nullptr;
           // ID3D12CommandQueue* commandQueue = nullptr;
            D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
            hr = device_->CreateCommandQueue(
                &commandQueueDesc,
                IID_PPV_ARGS(&commandQueue_));
            assert(SUCCEEDED(hr));
            //コマンドアロケーターの作成
            commandAllocator_=nullptr;
            //ID3D12CommandAllocator* commandAllocator = nullptr;
            hr = device_->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                IID_PPV_ARGS(&commandAllocator_));
            assert(SUCCEEDED(hr));
            //コマンドリストの作成
            commandList_ = nullptr;
            //ID3D12GraphicsCommandList* commandList = nullptr;
            hr =device_->CreateCommandList(
                0,
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                commandAllocator_.Get(), nullptr,
                IID_PPV_ARGS(&commandList_)
            );
            assert(SUCCEEDED(hr));
}
