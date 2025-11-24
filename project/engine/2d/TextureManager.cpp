#include "TextureManager.h"
#include "DXCommon.h"
TextureManager* TextureManager::instance=nullptr;

void TextureManager::Initialize(){

    textureDatas.reserve(DXCommon::kMaxSRVCount);
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