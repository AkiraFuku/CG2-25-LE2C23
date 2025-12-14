#pragma once
#include "Vector4.h"
#include<random>
#include<list>
#include "DrawFunction.h"

class DXCommon;
class SrvManager;
class TextureManager;
class ParticleManager
{
private:
     ParticleManager() = default;
    ~ParticleManager() = default;
    ParticleManager(ParticleManager&) = delete;
    ParticleManager& operator=(ParticleManager&) = delete;
    static ParticleManager* instance;
    DXCommon* dxCommon_=nullptr;
    SrvManager* srvManager_=nullptr;

    std::random_device seedGen_;
      std::mt19937 randomEngine_;
public:
    void Initialize(DXCommon* dxCommon,SrvManager* srvManager);
};

