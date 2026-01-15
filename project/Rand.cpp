#include "Rand.h"
#include <GameEngine.h>
#include <random>
#include <cassert>

void Rand::Initialize() {
	// メルセンヌ・ツイスターエンジンの初期化
	randomEngine.seed(seedGenerator_());
}

void Rand::RandomInitialize() { rotationDistribution = std::uniform_real_distribution<float>(-2.0f, 2.0f); }

float Rand::GetRandom() { return rotationDistribution(randomEngine); }