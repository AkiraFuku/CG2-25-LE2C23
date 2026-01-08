#pragma once  
#include "Obstacle.h"  

class Player;

class ObstacleSlow : public Obstacle
{
private:
    // 当たり判定サイズ  
    float width = 2.0f;
    float height = 2.0f;

public:

    void Initialize(Object3d* model, Camera* camera, const Vector3& position) override;
    void OnCollision(const Player* player) override;
};
