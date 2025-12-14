#include "PatricleEmitter.h"
#include "DXCommon.h"
#include "ParticleManager.h"
PatricleEmitter::PatricleEmitter(const  std::string name, Transform transfom, uint32_t count, float frequency, float frequencyTime) {
    transfom_ = transfom;
    count_ = count;
    frequency_ = frequency;
    frequencyTime_ = frequencyTime;
    name_ = name;

}
void PatricleEmitter::Update() {
    frequencyTime_ += DXCommon::kDeltaTime;
    if (frequency_ <= frequencyTime_)
    {
       ParticleManager::GetInstance()->Emit(name_,transfom_.translate,count_);
        frequencyTime_ -= frequency_;

    }
}