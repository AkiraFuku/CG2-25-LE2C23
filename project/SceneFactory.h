#pragma once
#include "AbstractSceneFactory.h"
class SceneFactory :public AbstractSceneFactory
{

    public:
        Scene* CreateScene(const std::string& sceneName) override;
};

