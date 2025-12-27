#include "TextureManager.h"
#include "DXCommon.h"
#include "StringUtility.h"
#include "SrvManager.h"
TextureManager* TextureManager::instance = nullptr;

uint32_t TextureManager::kSRVIndexTop = 1;

void TextureManager::Initialize(DXCommon* dxCommon, SrvManager* srvManager) {

    textureDatas.reserve(SrvManager::kMaxSRVCount);
    dxCommon_ = dxCommon;
    srvManager_ = srvManager;
}

TextureManager* TextureManager::GetInstance() {
    if (instance == nullptr)
    {
        instance = new TextureManager;
    }
    return instance;

};

void TextureManager::Finalize() {

    delete instance;
    instance = nullptr;
}
void TextureManager::LoadTexture(const std::string& filePath) {

    if (textureDatas.contains(filePath)) {
        return;
    }
    assert(srvManager_->IsMax());


    //テクスチャの読み込み
    DirectX::ScratchImage image{};
    std::wstring filePathW = StringUtility::ConvertString(filePath);
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
    //テクスチャデータ追加
    TextureData& textureData = textureDatas[filePath];
    textureData.metadata = mipImages.GetMetadata();//メタデータ
    textureData.resource = dxCommon_->CreateTextureResourse(textureData.metadata);//テクスチャリソース
    //SRVインデックス
    textureData.srvIndex = srvManager_->AllocateSRV();
    textureData.srvHandleCPU = srvManager_->GetCPUDescriptorHandle(textureData.srvIndex);
    textureData.srvHandleGPU = srvManager_->GetGPUDescriptorHandle(textureData.srvIndex);
  
    srvManager_->CreateSRVforTexture2D(textureData.srvIndex, textureData.resource.Get(), textureData.metadata.format, UINT(textureData.metadata.mipLevels));
    textureData.intermediateResource = dxCommon_->UploadTextureData(textureData.resource, mipImages);

}
void TextureManager::ReleaseIntermediateResources()
{
    for (auto& [key, textureData] : textureDatas) {
        if (textureData.intermediateResource) {
            // ComPtrのResetを呼び出してリソースを解放し、nullptrにする
            textureData.intermediateResource.Reset();
        }
    }
}

uint32_t TextureManager::GetTextureIndexByFilePath(const std::string& filePath)
{
    if (textureDatas.contains(filePath))
    {
        // 読み込み済みなら、保存されている srvIndex を返す
        return textureDatas[filePath].srvIndex;
    }

    //見つからなかった場合、その場で読み込む
    LoadTexture(filePath);



    // 3. 読み込んだ後のデータから srvIndex を返す
    return textureDatas[filePath].srvIndex;

}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHundleGPU(uint32_t textureindex)
{
    assert(textureindex < SrvManager::kMaxSRVCount);
    return srvManager_->GetGPUDescriptorHandle(textureindex);
}

const DirectX::TexMetadata& TextureManager::GetMetaData(const std::string& filePath)
{
    // 指定されたファイルパスのテクスチャが読み込まれているかチェック
   // 読み込まれていなければアサートで停止（または LoadTexture(filePath) を呼ぶ設計も可）
    assert(textureDatas.contains(filePath));

    // マップからデータを取得してメタデータを返す
    return textureDatas[filePath].metadata;

}
