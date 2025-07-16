#pragma once
class ID3D12Resource;

class ResourceObject
{
public:
    ResourceObject(ID3D12Resource* resource):resource_(resource){}
    ~ResourceObject();
    ID3D12Resource* Get() {return resource_;}
private:
    ID3D12Resource* resource_ ;
   
};

