#pragma once
#include "Object3d.h"
#include "Camera.h"
class Skydome {
private:
	WorldTransform worldTransform_; ///< ワールドトランスフォーム
	Object3d* model_ = nullptr;        ///< モデル
	Camera* camera_ = nullptr;      ///< カメラ
public:
	
	// 初期化
	void Initialize(Object3d*model,Camera*camera);
	// 更新
	void Update();
	// 描画
	void Draw();
};
