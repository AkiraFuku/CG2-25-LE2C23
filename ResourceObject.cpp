#include "ResourceObject.h"
#include<d3d12.h>



ResourceObject::~ResourceObject()
{
    if (resource_) {
        resource_->Release();
    }
}
