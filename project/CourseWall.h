#pragma once
#include "Camera.h"
#include "DrawFunction.h"
#include "MathFunction.h"
#include "Object3d.h"

#include "Vector4.h"

class CourseWall {
public:
  CourseWall() = default;
  ~CourseWall() = default;

  void Initialize(Object3d *model, Camera *camera, const Vector3 &position);

  void Update();
  void Draw();

  // 当たり判定用
  AABB GetAABB() const;

  // 便利（必要なら）
  const Vector3 &GetPosition() const { return transform_.translate; }

  void SetYaw(float yaw) { transform_.rotate.y = yaw; }
  void SetScale(const Vector3 &s) { transform_.scale = s; }

private:
  Transform transform_{{1, 1, 1}, {0, 0, 0}, {0, 0, 0}};

  Object3d *model_ = nullptr; // 借り物
  Camera *camera_ = nullptr;  // 借り物（今は使わなくてもOK）

  // AABBサイズ（幅/高さ/奥行き）※ワールド軸AABB
  Vector3 aabbSize_{3.0f, 3.0f, 3.0f};
};
