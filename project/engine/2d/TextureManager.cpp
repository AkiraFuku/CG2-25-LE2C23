#include "TextureManager.h"
#include "DXCommon.h"
#include "StringUtility.h"
TextureManager* TextureManager::instance=nullptr;

void TextureManager::Initialize( DXCommon* dxCommon){

    textureDatas.reserve(DXCommon::kMaxSRVCount);
    dxCommon_=dxCommon;
}

TextureManager* TextureManager::GetInstance(){
    if (instance==nullptr)
    {
        instance=new TextureManager;
    }
    return instance; 

};

void TextureManager::Finalize(){

    delete instance;
    instance=nullptr;
}
void TextureManager::LoadTexture(const std::string& filePath){

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
    textureDatas.resize(textureDatas.size()+1);
    TextureData& textureData=textureDatas.back();
    textureData.filePath=filePath;//ファイルパス
    textureData.metadata=mipImages.GetMetadata();//メタデータ
    textureData.resource=dxCommon_->CreateTextureResourse(textureData.metadata);//テクスチャリソース
    //SRVインデックス
    uint32_t srvIndex=static_cast<uint32_t>(textureDatas.size()-1);

    textureData.srvHandleCPU=dxCommon_->GetSRVCPUDescriptorHandle(srvIndex);
    textureData.srvHandleGPU=dxCommon_->GetSRVGPUDescriptorHandle(srvIndex);
     //metaDataを基にSRVの設定
    //
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = textureData. metadata.format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
    srvDesc.Texture2D.MipLevels = UINT( textureData.metadata.mipLevels);//最初のミップマップ
    // SRV
    dxCommon_->GetDevice()->CreateShaderResourceView(
        textureData.resource.Get(),
        &srvDesc,
        textureData.srvHandleCPU
    );


};