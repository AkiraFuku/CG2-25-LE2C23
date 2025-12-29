#include "SceneFactory.h"
#include "GameScene.h"
#include "TitleScene.h"
Scene* SceneFactory::CreateScene(const std::string& sceneName)
{
    Scene* scene = nullptr;
    if (sceneName == "TitleScene") {
         scene = new TitleScene();
    }
    else if (sceneName == "GameScene") {
         scene = new GameScene();
    }
    else if (sceneName == "ResultScene") {
     
        // scene = new ResultScene();
    }

    return scene;

}
