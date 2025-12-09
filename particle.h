#pragma once
#include "Vector4.h"
#include<random>
#include<list>

struct Particle
{
    Transform transfom;
    Vector3 velocity;
    Vector4 color;
    float lifeTime;
    float currentTime;

};

struct ParticleForGPU
{
    Matrix4x4 WVP;
    Matrix4x4 World;
    Vector4 color;

};

struct Emitter
{
    Transform transfom;//位置
    uint32_t count;//パーティクル数
    float frequency;//範囲
    float frequencyTime;//発生時間

};

Particle MakeNewParticle(std::mt19937& ramdamEngine );

std::list<Particle> Emit(const Emitter& emitter,std::mt19937& ramdamEngine);