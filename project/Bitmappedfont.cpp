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

    //sprite_[index_].at(0)->Update();

    if (!(*sprite_).empty()) {
      (*sprite_)[0]->Update();
    }
}

void Bitmappedfont::Update()
{
  if (sprite_ && !(*sprite_).empty() && index_ < (*sprite_).size()) {
    // 【修正】 (*sprite_) でベクター実体へアクセスし、[index_] で要素を取得
    (*sprite_)[index_]->Update();
  }
}

void Bitmappedfont::Draw()  
{  
    // ポインタの参照先のベクターが空でなく、インデックスが範囲内かチェック
  if (sprite_ && !(*sprite_).empty() && index_ < (*sprite_).size()) {
    (*sprite_)[index_]->Draw();
  }
}

Bitmappedfont::~Bitmappedfont()
{
}
