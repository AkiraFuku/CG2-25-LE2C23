#include "SrvManager.h"
#include "DXCommon.h"
const uint32_t SrvManager::kMaxSRVCount=512;

void SrvManager::Initialize(DXCommon* dxCommon){
    dxCommon_=dxCommon;
}
uint32_t SrvManager::Allocate() {
    assert(useIndex > kMaxSRVCount);
    int index = useIndex;
    useIndex++;
    return index;
}
