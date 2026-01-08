#pragma once
#define NOMINMAX
#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

// ===== あなたの環境に合わせて調整 =====
#include "Object3d.h"
#include "Vector4.h"
// =====================================

class CourseWall {
public:
  CourseWall() = default;
  ~CourseWall() = default;

  void Initialize(const std::string &modelName);
  void Clear();

  void Update();
  void Draw();

  // centerLine に沿って左右の壁を生成
  void BuildFromCenterLine(const std::vector<Vector3> &centerLine,
                           float courseHalfWidth, float wallHeight,
                           float wallThickness);

  // 球(車) vs カプセル(壁=線分+太さ) で押し戻し
  void ResolveSphere(Vector3 &pos, Vector3 &vel, float radius) const;

  // --- centerLine 作成ユーティリティ（GameSceneから呼ぶ想定） ---
  static void AddStraight(std::vector<Vector3> &out, const Vector3 &from,
                          const Vector3 &to, float step);

  static void AddArc(std::vector<Vector3> &out, const Vector3 &center,
                     float radius, float startRad, float endRad, int segments);

  bool IsCollision() const { return isCollision_; }

private:
  struct WallSegment {
    std::unique_ptr<Object3d> object;

    // ★カプセル（線分）情報（XZで判定）
    Vector3 a;    // world start
    Vector3 b;    // world end
    float halfT;  // thickness/2
    float height; // wall height
  };

  void AddWallPlane(const Vector3 &a, const Vector3 &b, const Vector3 &offset,
                    float wallHeight, float wallThickness);

private:
  std::string modelName_;
  std::vector<WallSegment> segments_;

  float blockLen_ = 1.0f;
  float blockRadius_ = 0.5f;

  mutable bool isCollision_ = false;
};
