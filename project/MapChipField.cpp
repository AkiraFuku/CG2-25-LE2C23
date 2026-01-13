#include "MapChipField.h"
#include "ModelManager.h"
#include <map>
#include <fstream>
#include <sstream>
#include <cassert>
#include <GameEngine.h>
#include <Framework.h>

namespace {

    // マップチップ対応表
std::map<char, MapChipType> mapChipTypeTable = {
    {'P',MapChipType::kPlayer},
    {'O',MapChipType::kObstacle},
};

}

void MapChipField::ResetMapChipData() {
	// マップチップデータをリセット
	mapChipData_.data.clear();
	mapChipData_.data.resize(kNumBlockVirtical);
	for (std::vector<MapChipDataUnit>& mapChipDataLine : mapChipData_.data) {
		mapChipDataLine.resize(kNumBlockHorizontal);
	}
}

void MapChipField::LoadMapChipCsv(const std::string& filePath) {
	// マップチップデータをリセット
	ResetMapChipData();

	// ファイルを開く
	std::ifstream file;
	file.open(filePath);
	assert(file.is_open());

	// マップチップCSV
	std::stringstream mapChipCsv;
	// ファイルの内容を文字列ストリームにコピー
	mapChipCsv << file.rdbuf();
	// ファイルを閉じる
	file.close();

	// CSVからマップチップデータを読み込む
	for (uint32_t i = 0; i < kNumBlockVirtical; ++i) {
		std::string line;
		getline(mapChipCsv, line);

		// 1行分の文字列をストリームに変換して解析しやすくする
		std::istringstream line_stream(line);

		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			std::string word;
			getline(line_stream, word, ',');

            // 空白の場合はスキップ
            if (word.empty()) {
                continue;
            }

            // 先頭の文字がマップチップ対応表に存在するか確認
            if (!mapChipTypeTable.contains(word[kChipType]))
            {
                continue;
            }

            // 先頭文字でマップチップのタイプを判別
            mapChipData_.data[i][j].type = mapChipTypeTable[word[kChipType]];

            // サブIDを含まない場合はスキップ
            if (word.size() <= kChipSubID) {
                continue;
            }

            // マップチップのサブIDを設定
            mapChipData_.data[i][j].subID = static_cast<uint8_t>(word[kChipSubID] - '0');
		}
	}
}

Vector3 MapChipField::GetMapChipPositionByIndex(uint32_t xIndex, uint32_t zIndex) { return Vector3(kBlockWidth * xIndex, 0 ,kBlockHeight * (kNumBlockVirtical - 1 - zIndex)); }

MapChipType MapChipField::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t zIndex) {
	if (xIndex < 0 || kNumBlockHorizontal - 1 < xIndex) {
		return MapChipType::kBlank;
	}

    return mapChipData_.data[zIndex][xIndex].type;
}

uint32_t MapChipField::GetNumBlockHorizontal() { return kNumBlockHorizontal; }

uint32_t MapChipField::GetNumBlockVirtical() { return kNumBlockVirtical; }

MapChipField::IndexSet MapChipField::GetMapChipIndexSetByPosition(const Vector3& position) { 
	IndexSet indexSet = {};
	indexSet.xIndex = static_cast<uint32_t>((position.x + (kBlockWidth / 2)) / kBlockWidth);
	indexSet.zIndex = static_cast<uint32_t>((position.z + (kBlockHeight / 2)) / kBlockHeight);
	indexSet.zIndex = kNumBlockVirtical - 1 - indexSet.zIndex;
	return indexSet;
}

MapChipField::Rect MapChipField::GetRectIndex(uint32_t xIndex, uint32_t zIndex) {
	// 指定したブロックの中心座標を取得する
	Vector3 center = GetMapChipPositionByIndex(xIndex, zIndex);

	Rect rect;
	rect.left = center.x - (kBlockWidth / 2.0f);
	rect.right = center.x + (kBlockWidth / 2.0f);
	rect.top = center.z + (kBlockHeight / 2.0f);
	rect.bottom = center.z - (kBlockHeight / 2.0f);

	return rect;
}




