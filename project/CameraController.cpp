#include "CameraController.h"
#include "CameraController.h"
// Playerの具体的な定義が必要なため、ここでインクルード
// ※まだ移植していない場合は仮のクラスかコメントアウトで対応してください
//#include "Player.h" 
#include <algorithm> // std::max, std::min

// 線形補間関数のヘルパー（自作エンジンのMathFunctionになければこれを使ってください）
Vector3 LerpShort(const Vector3& start, const Vector3& end, float t) {
    Vector3 result;
    result.x = start.x + (end.x - start.x) * t;
    result.y = start.y + (end.y - start.y) * t;
    result.z = start.z + (end.z - start.z) * t;
    return result;
}

void CameraController::Initialize(Camera* camera) {
    camera_ = camera;
    Reset();
}

void CameraController::Update() {
    if (!target_ || !camera_) return;

    // 1. ターゲットの場所を取得（Playerのメソッドに合わせて調整してください）
    // 例: Vector3 targetPos = target_->GetWorldPosition(); 
    // ここでは移植元のコードに合わせて Transform を取得する形を想定
    Vector3 targetPos = target_->GetWorldPosition(); 
    Vector3 targetVel = target_->GetVelocity();

    // 2. 目標座標の計算
    // destination = targetPos + offset + velocity * bias
    Vector3 velocityBias = { targetVel.x * kVelocityBias, targetVel.y * kVelocityBias, targetVel.z * kVelocityBias };
    
    // 自作エンジンのAdd関数などを使って計算（ここでは直感的に書きます）
    destination_.x = targetPos.x + targetOffset_.x + velocityBias.x;
    destination_.y = targetPos.y + targetOffset_.y + velocityBias.y;
    destination_.z = targetPos.z + targetOffset_.z + velocityBias.z;

    // 3. カメラの現在座標を取得
    Vector3 currentPos = camera_->GetTranslate();

    // 4. 線形補間 (Lerp) で滑らかに移動
    currentPos = LerpShort(currentPos, destination_, kInterpolationRate);

    // 5. 追従マージンの処理 (スクリーン端での押し出し)
    /* もし自作エンジンのVector3に演算子がなければ、
       要素ごとに比較・代入を行います。
    */
    if (currentPos.x < destination_.x + targetMargin_.left) {
        currentPos.x = destination_.x + targetMargin_.left;
    }
    if (currentPos.x > destination_.x + targetMargin_.right) {
        currentPos.x = destination_.x + targetMargin_.right;
    }
    if (currentPos.y < destination_.y + targetMargin_.bottom) {
        currentPos.y = destination_.y + targetMargin_.bottom;
    }
    if (currentPos.y > destination_.y + targetMargin_.top) {
        currentPos.y = destination_.y + targetMargin_.top;
    }

    // 6. 移動制限エリア (MoveArea) の適用
    currentPos.x = max(currentPos.x, moveArea_.left);
    currentPos.x = min(currentPos.x, moveArea_.right);
    currentPos.y = max(currentPos.y, moveArea_.bottom);
    currentPos.y = min(currentPos.y, moveArea_.top);

    // 7. 計算結果をカメラに適用
    camera_->SetTranslate(currentPos);
}

void CameraController::Reset() {
    if (target_ && camera_) {
        // 初期化時にターゲットの位置へ瞬時に移動
        Vector3 targetPos = target_->GetWorldPosition();
        Vector3 startPos;
        startPos.x = targetPos.x + targetOffset_.x;
        startPos.y = targetPos.y + targetOffset_.y;
        startPos.z = targetPos.z + targetOffset_.z;
        camera_->SetTranslate(startPos);
        destination_ = startPos;
    }
}