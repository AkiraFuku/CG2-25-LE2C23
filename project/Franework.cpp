#include "Franework.h"

void Franework::Run()
{
    Initialize();
    while (true) {
        Update();
        if (IsEnd()) {
            break;
        }
        Draw();
    }
    Finalize();
}
