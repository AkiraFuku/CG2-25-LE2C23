#pragma once
#include <wrl.h>
#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cstdint> // uint32_t用

class Audio
{
public:
    // ハンドル型の定義（中身はただの数値）
    using SoundHandle = uint32_t;

    // 音声データ構造体
    struct SoundData
    {
        WAVEFORMATEX wfex;
        std::vector<BYTE> buffer;
    };

    static Audio* GetInstance();
       friend struct std::default_delete<Audio>;
    void Initialize();
    void Finalize();

    // 音声データの読み込み
    // 戻り値：管理用ハンドル（失敗時や無効時は -1 を返すなどのルールにすると良い）
    SoundHandle LoadAudio(const std::string& filename);

    // 音声データの解放
    void UnloadAudio(SoundHandle soundHandle);

    // 音声再生
    // handle: LoadWaveで取得したハンドル
    // loop: ループ再生するか
    // volume: 音量 (0.0f ~ 1.0f以上)
    void PlayAudio(SoundHandle soundHandle, bool loop = false, float volume = 1.0f);

private:
    Audio() = default;
    ~Audio() = default;
    Audio(const Audio&) = delete;
    Audio& operator=(const Audio&) = delete;

    Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
    IXAudio2MasteringVoice* masteringVoice_ = nullptr;

    // ハンドルと音声データの紐づけ管理
    std::map<SoundHandle, SoundData> soundDatas_;
    
    // 次に割り当てるハンドルの番号
    SoundHandle nextHandle_ = 0u;

    // 再生中のVoiceリスト（リーク対策用・簡易版）
    std::vector<IXAudio2SourceVoice*> activeVoices_;

    static std::unique_ptr<Audio> instance;
};