#pragma once
#include"MathFunction.h"
#include "Sprite.h"
#include"Object3D.h"
#include "Model.h"
#include "Camera.h"
#include "ParicleEmitter.h"
#include "Audio.h"
#include "TextureManager.h"
#include "Scene.h"
#include <memory>
#include "Player.h"

class GameScene :public Scene
{
public:
    void Initialize() override;
    void Finalize()override;
    void Update()override;
    void Draw()override;
private:
    std::unique_ptr<Camera> camera;
    std::unique_ptr<Sprite> sprite;
    std::unique_ptr<Object3d> object3d2;
    std::unique_ptr<Object3d> object3d;
    std::unique_ptr<ParicleEmitter> emitter;
    Audio::SoundData soundData1;

    // 自キャラ
    std::unique_ptr<Player> player_;
    // プレイヤーのモデル
    std::unique_ptr<Object3d> playerModel_;
};

