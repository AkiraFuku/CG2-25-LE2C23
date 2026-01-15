#include "Bitmappedfont.h"
#include "ModelManager.h"
#include <GameEngine.h>
#include <Framework.h>
#include <cassert>

Bitmappedfont::Bitmappedfont() = default;

void Bitmappedfont::Initialize(std::vector<std::unique_ptr<Sprite>>* sprite, Camera* camera)
{
    // nullポインタチェック
    assert(sprite);
    assert(camera);

    // 引数をメンバ変数に記録
    sprite_ = sprite;
    camera_ = camera;

    sprite_[index_].at(0)->Update();
}

void Bitmappedfont::Update()
{
    if (!sprite_[index_].empty()) {
        sprite_[index_].at(0)->Update(); // その中のスプライトを更新
    }
}

void Bitmappedfont::Draw()  
{  
    if (!sprite_[index_].empty()) {
        sprite_[index_].at(0)->Draw();
    }
}

Bitmappedfont::~Bitmappedfont()
{
}
