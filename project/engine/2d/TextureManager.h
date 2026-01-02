#pragma once
#include <wrl.h>
#include <string>
#include <unordered_map>
#include<d3d12.h>
#include"DirectXTex.h"
#include"d3dx12.h"

class DXCommon;
class SrvManager;
class TextureManager
{
private:

    //テクスチャデータ構造体
    struct TextureData
    {
        uint32_t srvIndex;
        DirectX::TexMetadata metadata;
        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;
        D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;
        Microsoft::WRL::ComPtr<ID3D12Resource>intermediateResource;
    };

    TextureManager() = default;
    ~TextureManager() = default;
    TextureManager(TextureManager&) = delete;
    TextureManager& operator=(TextureManager&) = delete;
    std::unordered_map<std::string, TextureData> textureDatas;

    DXCommon* dxCommon_ = nullptr;
    SrvManager* srvManager_ = nullptr;

    static uint32_t kSRVIndexTop;
public:
    static std::unique_ptr<TextureManager> instance;

    void Initialize(DXCommon* dxCommon, SrvManager* srvManager);
    static TextureManager* GetInstance();
    void Finalize();
    //テクスチャロード
    void LoadTexture(const std::string& filePath);
    //中間リソース解放
    void ReleaseIntermediateResources();
    //SRVindex開始番号取得
    uint32_t GetTextureIndexByFilePath(const std::string& filePath);
    //テクスチャ番号からGPUハンドル
    D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHundleGPU(uint32_t textureindex);
    //メタデータ
    const DirectX::TexMetadata& GetMetaData(const std::string& filePath);
};

