#pragma once
#include "Vector4.h"
#include<random>

struct Particle
{
    Transform transfom;
    Vector3 velocity;
    Vector4 color;

};

struct ParticleForGPU
{
    Matrix4x4 WVP;
    Matrix4x4 World;
    Vector4 color;

};

Particle MakeNewParticle(std::mt19937& ramdamEngine);