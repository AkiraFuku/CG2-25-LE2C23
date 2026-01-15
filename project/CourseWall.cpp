#define NOMINMAX
#include "CourseWall.h"

void CourseWall::Initialize(Object3d *model, Camera *camera,
                            const Vector3 &position) {
  model_ = model;
  camera_ = camera;

  transform_.translate = position;

}

void CourseWall::Update() {
  if (!model_) {
    return;
  }

  model_->SetScale(transform_.scale);
  model_->SetRotate(transform_.rotate);
  model_->SetTranslate(transform_.translate);
  model_->Update();
}


void CourseWall::Draw() {
  if (!model_) {
    return;
  }
  model_->Draw();
}

AABB CourseWall::GetAABB() const {
  Vector3 half = {aabbSize_.x * 0.5f, aabbSize_.y * 0.5f, aabbSize_.z * 0.5f};

  AABB aabb{};
  aabb.min = {transform_.translate.x - half.x, transform_.translate.y - half.y,
              transform_.translate.z - half.z};
  aabb.max = {transform_.translate.x + half.x, transform_.translate.y + half.y,
              transform_.translate.z + half.z};
  return aabb;
}
