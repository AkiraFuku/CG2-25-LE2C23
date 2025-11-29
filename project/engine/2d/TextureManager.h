#pragma once
#include <wrl.h>
#include <string>
#include <vector>
#include<d3d12.h>
#include"externals/DirectXTex/DirectXTex.h"
#include"externals/DirectXTex/d3dx12.h"

class DXCommon;

class TextureManager
{
private:

    //テクスチャデータ構造体
    struct TextureData
    {
        std::string filePath;
        DirectX::TexMetadata metadata;
        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;
        D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;
        Microsoft::WRL::ComPtr<ID3D12Resource>intermediateResource;
    };

    static TextureManager* instance;
    TextureManager() = default;
    ~TextureManager() = default;
    TextureManager(TextureManager&) = delete;
    TextureManager& operator=(TextureManager&) = delete;
    std::vector<TextureData> textureDatas;

    DXCommon* dxCommon_=nullptr;

    static uint32_t kSRVIndexTop;
public:
    void Initialize( DXCommon* dxCommon);
    static TextureManager* GetInstance();
    void Finalize();
    //テクスチャロード
    void LoadTexture(const std::string& filePath);
    //中間リソース解放
    void ReleaseIntermediateResources();
};

