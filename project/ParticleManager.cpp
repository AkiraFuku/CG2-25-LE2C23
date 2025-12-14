#include "ParticleManager.h"
#pragma once
ParticleManager* ParticleManager::instance = nullptr;

void ParticleManager::Initialize(DXCommon* dxCommon,SrvManager* srvManager){

    dxCommon_=dxCommon;
    srvManager_=srvManager;


}