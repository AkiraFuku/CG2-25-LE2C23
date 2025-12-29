#include "GameEngine.h"
//winmain
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    GameEngine gameEngine;
    gameEngine.Initialize();
    //メインループ
    while (true) {
        gameEngine.Update();
        if (gameEngine.IsEnd()) {
            break;
        }
        gameEngine.Draw();
    }
    gameEngine.Finalize();
    return 0;
}