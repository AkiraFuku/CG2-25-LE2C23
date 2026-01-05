#pragma once
#include <wrl.h>
#include <fstream>
#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")

#include <string>
#include <vector>

class Audio
{
public:
    struct ChunkHeader
    {
        char id[4];      // チャンクID（例: "fmt "や"data"など）
        int32_t size;   // チャンクサイズ
    };
    struct RiffHeader
    {
        ChunkHeader chunk;
        char type[4]; // RIFFタイプ（例: "WAVE"）
    };

    struct FormatChunk
    {
        ChunkHeader chunk;
        WAVEFORMATEX fmt; // WAVEフォーマット情報
    };

    struct SoundData
    {
        //波形フォーマット
        WAVEFORMATEX wfex; // 波形フォーマット
        //バッファ
        std::vector<BYTE> buffer; // 音声データバッファ
    };

     
    // シングルトンインスタンスの取得
    static Audio* GetInstance();

    // 初期化
    void Initialize();
    void Finalize();
    
    // 音声データの読み込み (メンバ関数に変更)
    SoundData SoundLoadWave(const std::string& filename);

    // 音声再生
    void PlayAudio(const SoundData& soundData);
   static std::unique_ptr<Audio> instance;
   friend struct std::default_delete<Audio>;

private:
    // コンストラクタ・デストラクタをprivateにする（外部からの生成禁止）
    Audio() = default;
    ~Audio()=default;

    // コピーコンストラクタと代入演算子を無効化（コピー禁止）
    Audio(const Audio&) = delete;
    Audio& operator=(const Audio&) = delete;

    void SetSoundData(const SoundData& soundData) {
        soundData_ = soundData;
    }
    void SoundUnload(SoundData* soundData);
    void SoundPlayWave(IXAudio2* xAudio2, const SoundData& SoundData);

    Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
    IXAudio2MasteringVoice* masteringVoice_ = nullptr; // マスターボイス
    SoundData soundData_; // 音声データ
};