#pragma once
#include "Object3d.h"
#include <memory>
class SkyDome
{
    public:
    void Initialize();
    void Update();
    void Draw();

private:
    // 自作エンジンのObject3dを使用
    std::unique_ptr<Object3d> object_;
};

