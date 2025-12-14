#pragma once
class DXCommon;
class SrvManager
{
public:

    void Initialize(DXCommon* dxCommon);
private:
    DXCommon* dxCommon_=nullptr;
};

