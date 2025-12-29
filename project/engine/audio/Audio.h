#pragma once
#include <wrl.h>
#include<fstream>

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
    ~Audio();
  
    void Initialize();
  static  SoundData SoundLoadWave(const std::string& filename);

    
    void PlayAudio(const SoundData& soundData );


    private:
        void SetSoundData(const SoundData& soundData) {
         soundData_ = soundData;
        }
        void SoundUnload(SoundData* soundData);
        void SoundPlayWave(IXAudio2* xAudio2,const SoundData& SoundData);
        Microsoft::WRL::ComPtr<IXAudio2> xAudio2_ ;
        IXAudio2MasteringVoice* masteringVoice_; // マスターボイス
        SoundData soundData_; // 音声データ

};

