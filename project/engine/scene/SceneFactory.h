#pragma once
#include "AbstractSceneFactory.h"
class SceneFactory :public AbstractSceneFactory
{

public:
    std::unique_ptr<Scene> CreateScene(const std::string& sceneName) override;
};

