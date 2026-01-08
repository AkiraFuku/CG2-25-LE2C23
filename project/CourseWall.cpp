#define NOMINMAX
#include "CourseWall.h"

// ★追加：MathFunction を使う
#include "MathFunction.h"

#include <algorithm>
#include <cmath>

namespace {

// XZで左法線（a->b の左側）
// ※dir の長さが0でも Normalize が死なない想定（もし死ぬなら eps ガードする）
inline Vector3 LeftNormalXZ(const Vector3 &dir) {
  Vector3 d = dir;
  d.y = 0.0f;
  d = Normalize(d);                           // MathFunction
  return Normalize(Vector3{-d.z, 0.0f, d.x}); // left = (-z,0,x)
}

} // namespace

void CourseWall::Initialize(const std::string &modelName) {
  modelName_ = modelName;
  segments_.clear();
}

void CourseWall::Clear() { segments_.clear(); }

void CourseWall::Update() {
  for (auto &s : segments_) {
    if (s.object)
      s.object->Update();
  }
}

void CourseWall::Draw() {
  for (auto &s : segments_) {
    if (s.object)
      s.object->Draw();
  }
}

void CourseWall::BuildFromCenterLine(const std::vector<Vector3> &centerLine,
                                     float courseHalfWidth, float wallHeight,
                                     float wallThickness) {
  segments_.clear();
  if (centerLine.size() < 2)
    return;

  for (size_t i = 0; i + 1 < centerLine.size(); ++i) {
    const Vector3 a = centerLine[i];
    const Vector3 b = centerLine[i + 1];

    Vector3 dir = b - a;
    dir.y = 0.0f;

    // 左右オフセット（センターラインからの横方向）
    const Vector3 nL = LeftNormalXZ(dir);
    const Vector3 offsetL = nL * courseHalfWidth;
    const Vector3 offsetR = nL * -courseHalfWidth;

    AddWallPlane(a, b, offsetL, wallHeight, wallThickness);
    AddWallPlane(a, b, offsetR, wallHeight, wallThickness);
  }
}

void CourseWall::AddWallPlane(const Vector3 &a, const Vector3 &b,
                              const Vector3 &offset, float wallHeight,
                              float wallThickness) {
  // 壁線分（ワールド）
  Vector3 a2 = a + offset;
  Vector3 b2 = b + offset;

  Vector3 seg = b2 - a2;
  seg.y = 0.0f;

  // segLen = |seg|
  const float segLen = std::max(Length(seg), 0.0f); // MathFunction
  if (segLen <= 1.0e-4f)
    return;

  const Vector3 dir = seg / segLen; // operator/(Vector3,float) がある前提

  // yaw（Z+前提）
  const float yaw = std::atan2(dir.x, dir.z);

  // ---- 「引き延ばさず」ブロックを並べる ----
  const int count = std::max(1, (int)std::floor(segLen / blockLen_));
  const float step = segLen / (float)count;

  const float capR = std::max(blockRadius_, wallThickness * 0.5f);

  for (int i = 0; i < count; ++i) {
    // ブロック中心（線分上）
    const float t = (i + 0.5f) * step;
    Vector3 mid = a2 + dir * t;
    mid.y += wallHeight * 0.5f;

    WallSegment ws{};
    ws.object = std::make_unique<Object3d>();
    ws.object->Initialize();
    ws.object->SetModel(modelName_);

    // 見た目：元サイズ（X,Zは1.0）、高さだけ欲しければYだけ伸ばす
    ws.object->SetTranslate(mid);
    ws.object->SetRotate(Vector3{0.0f, yaw, 0.0f});
    ws.object->SetScale(Vector3{1.0f, wallHeight, 1.0f});

    // 判定用カプセル：ブロック1個分を短い線分にする（XZ）
    const Vector3 half = dir * (step * 0.5f);

    // yは判定で使わないけど、形式上そろえる
    const Vector3 baseMid = mid + Vector3{0.0f, -wallHeight * 0.5f, 0.0f};
    ws.a = baseMid - half;
    ws.b = baseMid + half;

    ws.halfT = capR; // ★半径
    ws.height = wallHeight;

    segments_.push_back(std::move(ws));
  }
}

// ★ここが「球 vs カプセル（線分＋太さ）」の本体
void CourseWall::ResolveSphere(Vector3 &pos, Vector3 &vel, float radius) const {

    //isCollision_ = false;

  for (const auto &w : segments_) {

    // 高さ方向（簡易）
    if (pos.y > w.height + radius) {
      continue;
    }

    // 線分 a-b の最近点（XZのみ）
    Vector3 a = w.a;
    a.y = 0.0f;
    Vector3 b = w.b;
    b.y = 0.0f;
    Vector3 p = pos;
    p.y = 0.0f;

    const Vector3 ab = b - a;
    const Vector3 ap = p - a;

    const float abLenSq = ab.x * ab.x + ab.z * ab.z;
    if (abLenSq <= 1.0e-8f) {
      continue;
    }

    float t = (ap.x * ab.x + ap.z * ab.z) / abLenSq;
    t = std::clamp(t, 0.0f, 1.0f);

    const Vector3 closest{a.x + ab.x * t, 0.0f, a.z + ab.z * t};

    Vector3 d = p - closest;
    d.y = 0.0f;

    const float hitR = radius + w.halfT;
    const float distSq = d.x * d.x + d.z * d.z;
    if (distSq >= hitR * hitR) {
      continue;
    }

    isCollision_ = true;

    const float dist = std::sqrt(std::max(distSq, 1.0e-8f));
    const float pen = hitR - dist;

    Vector3 n;
    if (dist > 1.0e-6f) {
      n = Vector3{d.x / dist, 0.0f, d.z / dist};
    } else {
      n = Vector3{1.0f, 0.0f, 0.0f};
    }

    // 位置補正（XZ）
    pos.x += n.x * pen;
    pos.z += n.z * pen;

    // 速度：壁に向かう成分だけ除去（滑る）
    const float vn = vel.x * n.x + vel.z * n.z;
    if (vn < 0.0f) {
      vel.x -= n.x * vn;
      vel.z -= n.z * vn;
    }
  }
}

void CourseWall::AddStraight(std::vector<Vector3> &out, const Vector3 &from,
                             const Vector3 &to, float step) {
  if (step <= 0.0f)
    step = 1.0f;

  Vector3 d = to - from;
  d.y = 0.0f;

  const float len = std::max(Length(d), 0.0f); // MathFunction
  const int count = std::max(1, (int)(len / step));

  for (int i = 0; i <= count; ++i) {
    const float t = (float)i / (float)count;
    out.push_back(Vector3{
        from.x + (to.x - from.x) * t,
        from.y + (to.y - from.y) * t,
        from.z + (to.z - from.z) * t,
    });
  }
}

void CourseWall::AddArc(std::vector<Vector3> &out, const Vector3 &center,
                        float radius, float startRad, float endRad,
                        int segments) {
  segments = std::max(1, segments);

  for (int i = 0; i <= segments; ++i) {
    const float t = (float)i / (float)segments;
    const float a = startRad + (endRad - startRad) * t;

    out.push_back(Vector3{
        center.x + std::cos(a) * radius,
        center.y,
        center.z + std::sin(a) * radius,
    });
  }
}
