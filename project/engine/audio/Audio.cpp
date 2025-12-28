#include "Audio.h"
#include <cassert>
#include <mfapi.h>
#pragma comment(lib,"mfplat.lib")
#include <mfreadwrite.h>
#pragma comment(lib,"mfreadwrite.lib")
#include "StringUtility.h"
#pragma comment(lib,"mfuuid.lib")
#include <wrl.h>
using namespace Microsoft::WRL;
Audio::~Audio()
{
    HRESULT result;
    result = MFShutdown();
    assert(SUCCEEDED(result));
    //xAudio2_.Reset();
    //SoundUnload(&soundData_); // 音声データの解放
}
void Audio::Initialize()
{
    HRESULT result;
    // 
    result = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
    assert(SUCCEEDED(result));
    //// マスターボイスの作成
    //result = xAudio2_->CreateMasteringVoice(&masteringVoice_);
    //assert(SUCCEEDED(result));
}

Audio::SoundData Audio::SoundLoadFile(const  std::string& filename)
{
    //ファイルパスをwstringに変換
    std::wstring filePathW = StringUtility::ConvertString(filename);
    HRESULT result;
    //ソースリーダーの生成
    ComPtr<IMFSourceReader> pReader;
    result = MFCreateSourceReaderFromURL(filePathW.c_str(), NULL, &pReader);
    assert(SUCCEEDED(result));
    //出力メディアタイプの設定
    ComPtr<IMFMediaType> pPCMType;
    result = MFCreateMediaType(&pPCMType);
    pPCMType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    pPCMType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    result = pReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, NULL, pPCMType.Get());
    assert(SUCCEEDED(result));


    ////ファイルオープン
    //std::ifstream file;
    //file.open(filename, std::ios_base::binary);

    //assert(file.is_open());

    ////wavdataの読み込み
    //RiffHeader riff;
    //file.read((char*)&riff, sizeof(riff));
    //if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
    //    assert(0);
    //}
    //if (strncmp(riff.type, "WAVE", 4) != 0) {
    //    assert(0);
    //}

    //FormatChunk format = {};

    //file.read((char*)&format, sizeof(ChunkHeader));
    //if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
    //    assert(0);
    //}
    //assert(format.chunk.size <= sizeof(format.fmt));
    //file.read((char*)&format.fmt, format.chunk.size);
    //ChunkHeader data;
    //file.read((char*)&data, sizeof(data));

    //if (strncmp(data.id, "JUNK", 4) == 0) {
    //    file.seekg(data.size, std::ios_base::cur); // JUNKチャンクをスキップ
    //    file.read((char*)&data, sizeof(data)); // 次のチャンクを読み込む
    //}
    //if (strncmp(data.id, "data", 4) != 0) {
    //    assert(0);
    //}

    //char* pBuffer = new char[data.size];
    //file.read(pBuffer, data.size);
    ////ファイルを閉じる
    //file.close();
    ////読み込んだデータを構造体に格納
    //SoundData soundData{};

    //soundData.wfex = format.fmt; // 波形フォーマット
    //soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer); // バッファ先頭アドレス
    //soundData.bufferSize = data.size; // バッファサイズ

    //return soundData;
}

void Audio::PlayAudio(const SoundData& soundData)
{
    SetSoundData(soundData); // 音声データを設定
    SoundPlayWave(xAudio2_.Get(), soundData_); // 音声の再生
}

void Audio::SoundUnload(SoundData* soundData)
{
    delete[] soundData->pBuffer; // バッファの解放
    soundData->pBuffer = 0; // ポインタをnullptrに設定
    soundData->bufferSize = 0; // バッファサイズを0に設定
    soundData->wfex = {}; // 波形フォーマットを初期化
}

void Audio::SoundPlayWave(IXAudio2* xAudio2, const SoundData& SoundData)
{
    HRESULT result;
    IXAudio2SourceVoice* pSourceVoice = nullptr;
    result = xAudio2->CreateSourceVoice(&pSourceVoice, &SoundData.wfex);
    assert(SUCCEEDED(result));

    //音声データの再生
    XAUDIO2_BUFFER buf = {};
    buf.pAudioData = SoundData.pBuffer; // 音声データのポインタ
    buf.AudioBytes = SoundData.bufferSize; // 音声データのサイズ
    buf.Flags = XAUDIO2_END_OF_STREAM; // ストリームの終端を示すフラグ

    result = pSourceVoice->SubmitSourceBuffer(&buf);
    result = pSourceVoice->Start(); // 音声の再生を開始
}
