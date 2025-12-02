#pragma once
#include "Vector4.h"
#include<random>

struct Particle
{
    Transform transfom;
    Vector3 velocity;

};

Particle MakeNewParticle(std::mt19937& ramdamEngine);