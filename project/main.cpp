#include "GameEngine.h"
//winmain
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Framework* game = new GameEngine();
    game->Run();
    delete game;
    return 0;
}