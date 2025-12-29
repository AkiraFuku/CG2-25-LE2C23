#include "GameEngine.h"
//winmain
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Framwork* game=new GameEngine();
   
    game->Run();
    delete game;
    return 0;
}