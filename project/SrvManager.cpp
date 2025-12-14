#include "SrvManager.h"
#include "DXCommon.h"
const uint32_t SrvManager::kMaxSRVCount=512;

void SrvManager::Initialize(DXCommon* dxCommon){
    dxCommon_=dxCommon;
}