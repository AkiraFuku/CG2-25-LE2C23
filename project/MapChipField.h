#pragma once
#include"MathFunction.h"
#include "DrawFunction.h"
#include"Object3D.h"
#include "Model.h"
#include "Camera.h"
#include <memory>
#include <string>
#include <vector>

// マップチップ種別
enum class MapChipType {
	kBlank,
    kPlayer,
    kObstacle,
};

// 1マス分のデータ
struct  MapChipDataUnit
{
    MapChipType type; // マップチップの種別
    uint8_t subID;    // 種類ごとのサブID
};

// ステージ全体のマップチップデータ
struct MapChipData {
	std::vector<std::vector<MapChipDataUnit>> data;
};

// マップチップCSVの文字番号
enum MapChipCharIndex {
    kChipType = 0,
    kChipSubID = 1,
};

class MapChipField {
private:
	// 1ブロックのサイズ
	static inline const float kBlockWidth = 2.0f;
	static inline const float kBlockHeight = 2.0f;

	// ブロックの個数
	static inline const uint32_t kNumBlockVirtical = 20;
	static inline const uint32_t kNumBlockHorizontal = 26;

	MapChipData mapChipData_;

	
public:

	struct IndexSet {
		uint32_t xIndex;
		uint32_t zIndex;
	};
	
	// 範囲矩形
	struct Rect {
		float left;
		float right;
		float top;
		float bottom;
	};

	void ResetMapChipData();
	void LoadMapChipCsv(const std::string& filePath);
	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t zIndex);
	Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t zIndex);
	uint32_t GetNumBlockHorizontal();
	uint32_t GetNumBlockVirtical();
	IndexSet GetMapChipIndexSetByPosition(const Vector3& position);
	Rect GetRectIndex(uint32_t xIndex, uint32_t zIndex);
    uint8_t GetMapChipSubIDByIndex(uint32_t xIndex, uint32_t zIndex) {
        return mapChipData_.data[zIndex][xIndex].subID;
    }
};
