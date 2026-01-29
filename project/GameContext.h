#pragma once

// シーン間で共有したいデータを管理するクラス
class GameContext {
public:
  // シングルトンインスタンスの取得
  static GameContext *GetInstance();

  // ステージ番号のセッター・ゲッター
  void SetStageNum(int stageNum) { stageNum_ = stageNum; }
  int GetStageNum() const { return stageNum_; }

private:
  GameContext() = default;
  ~GameContext() = default;
  GameContext(const GameContext &) = delete;
  const GameContext &operator=(const GameContext &) = delete;

  // 現在選択されているステージ番号（初期値1）
  int stageNum_ = 1;
};