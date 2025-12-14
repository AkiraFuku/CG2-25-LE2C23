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
    assert(textureDatas.size() + kSRVIndexTop < SrvManager::kMaxSRVCount);


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
//    uint32_t srvIndex = static_cast<uint32_t>(textureDatas.size() - 1) + kSRVIndexTop;
    textureData.srvIndex = srvManager_->Allocate();
    textureData.srvHandleCPU = srvManager_->GetCPUDescriptorHandle(textureData.srvIndex);
    textureData.srvHandleGPU = srvManager_->GetGPUDescriptorHandle(textureData.srvIndex);
    //metaDataを基にSRVの設定
   //
   //D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
   //srvDesc.Format = textureData. metadata.format;
   //srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
   //srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
   //srvDesc.Texture2D.MipLevels = UINT( textureData.metadata.mipLevels);//最初のミップマップ
   //// SRV
   //dxCommon_->GetDevice()->CreateShaderResourceView(
   //    textureData.resource.Get(),
   //    &srvDesc,
   //    textureData.srvHandleCPU
   //);
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
    assert(textureindex < textureDatas.size());
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
