#pragma once
#include "Vector4.h"
class PatricleEmitter
{
    PatricleEmitter(const  std::string name, Transform transfom,uint32_t count,float frequency,float frequencyTime);
    void Update();
private:
        Transform transfom_;//位置
        uint32_t count_;//パーティクル数
       float frequency_;//範囲
       float frequencyTime_;//発生時間
        std::string name_;
};

