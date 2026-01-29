#include "GameContext.h"

GameContext *GameContext::GetInstance() {
  static GameContext instance;
  return &instance;
}