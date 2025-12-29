#pragma once

#include"Scene.h"
#include <string>

class AbstractSceneFactory{

    public:
    virtual ~AbstractSceneFactory() = default;
    virtual Scene* CreateScene(const std::string& sceneName) = 0;
};