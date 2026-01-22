#pragma once
#include "Vector4.h"
#include "ParticleManager.h"
class ParicleEmitter {
public:
  enum class Type {
    kNormal,
    kBubble,
  };

public:
  ParicleEmitter(const std::string name, Transform transfom, uint32_t count,
                 float frequency, float frequencyTime, Type type);
    void Update();
    void Emit();
private:
    Transform transfom_;//位置
    uint32_t count_;//パーティクル数
    float frequency_;//範囲
    float frequencyTime_;//発生時間
    std::string name_;
    Type type_ = Type::kNormal;
};

