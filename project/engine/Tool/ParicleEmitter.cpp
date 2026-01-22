#include "ParicleEmitter.h"
#include "DXCommon.h"

ParicleEmitter::ParicleEmitter(const std::string name, Transform transfom,
                               uint32_t count, float frequency,
                               float frequencyTime, Type type) {
  transfom_ = transfom;
  count_ = count;
  frequency_ = frequency;
  frequencyTime_ = frequencyTime;
  name_ = name;
  type_ = type;
}
void ParicleEmitter::Update() {
  frequencyTime_ += DXCommon::kDeltaTime;
  if (frequency_ <= frequencyTime_) {
    Emit();
    frequencyTime_ -= frequency_;
  }
}

void ParicleEmitter::Emit() {
  switch (type_) {
  case Type::kNormal:

    ParticleManager::GetInstance()->Emit(name_, transfom_.translate, count_);

    break;
  case Type::kBubble:

    ParticleManager::GetInstance()->EmitBubble(name_, transfom_.translate,
                                               count_);

    break;
  }
}
