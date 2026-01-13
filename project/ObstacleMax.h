#pragma once
#include "Obstacle.h"

class ObstacleMax :
    public Obstacle
{
private:

public:

    void Initialize(Object3d* model, Camera* camera, const Vector3& position) override;
    void OnCollision(const Player* player) override;

};

