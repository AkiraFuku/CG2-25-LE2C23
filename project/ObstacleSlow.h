#pragma once  
#include "Obstacle.h"  

class Player;

class ObstacleSlow : public Obstacle
{
private:
   
public:

    void Initialize(Object3d* model, Camera* camera, const Vector3& position, Player* player) override;
    void OnCollision(const Player* player) override;
};
