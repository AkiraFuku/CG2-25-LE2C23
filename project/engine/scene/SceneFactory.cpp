#include "SceneFactory.h"
#include "GameScene.h"
#include "TitleScene.h"
#include "StageSelectScene.h"

#include <iostream>
#include <string>


constexpr unsigned int hash(const char* s, int off = 0) {
    return !s[off] ? 5381 : (hash(s, off+1)*33) ^ s[off];
}

std::unique_ptr<Scene> SceneFactory::CreateScene(const std::string& sceneName)
{
    std::unique_ptr<Scene> scene = nullptr;

    switch (hash(sceneName.c_str()))
    {
   case hash("TitleScene"):
        scene = std::make_unique<TitleScene>();
            break;
        case hash("GameScene"):
          scene = std::make_unique<GameScene>();
            break;
        case hash("SelectScene"):
          scene = std::make_unique<StageSelectScene>();
            break;
        default:
            std::cout << "不明なシーンです" << std::endl;
            break;
    }

    return scene;
}