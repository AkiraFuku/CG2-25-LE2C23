#include "GameEngine.h"
#include <memory>
//winmain
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    std::unique_ptr<Framework> game = std::make_unique<GameEngine>();
    game->Run();

    return 0;
}