#include "Audio.h"
#include <cassert>
#include "StringUtility.h" // 既存の変換クラス
#include <windows.h>

// Media Foundation 関連
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfuuid.lib") 
#pragma comment(lib, "mfreadwrite.lib")

using namespace Microsoft::WRL;

std::unique_ptr<Audio> Audio::instance = nullptr;

Audio* Audio::GetInstance()
{
    if (instance == nullptr)
    {
        instance.reset(new Audio());
    }
    return instance.get();
}

void Audio::Initialize()
{
    HRESULT result;
    // MF初期化
    result = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
    assert(SUCCEEDED(result));

    // XAudio2初期化
    result = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
    assert(SUCCEEDED(result));

    // マスターボイス作成
    result = xAudio2_->CreateMasteringVoice(&masteringVoice_);
    assert(SUCCEEDED(result));
}

void Audio::Finalize()
{
    // 再生中のボイスを破棄
  // 再生中のボイスをすべて破棄
    for (auto& voice : activeVoices_)
    {
        if (voice.sourceVoice)
        {
            voice.sourceVoice->Stop();
            voice.sourceVoice->FlushSourceBuffers();
            voice.sourceVoice->DestroyVoice();
            voice.sourceVoice = nullptr;
        }
    }
    activeVoices_.clear();

    // マスターボイス破棄
    if (masteringVoice_) {
        masteringVoice_->DestroyVoice();
        masteringVoice_ = nullptr;
    }

    xAudio2_.Reset();
    soundDatas_.clear(); // データ解放
    MFShutdown();
}
void Audio::Update()
{
    for (auto it = activeVoices_.begin(); it != activeVoices_.end(); )
    {
        XAUDIO2_VOICE_STATE state{};
        it->sourceVoice->GetState(&state);

        if (state.BuffersQueued == 0)
        {
            it->sourceVoice->DestroyVoice();
            it = activeVoices_.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

// 読み込み：ハンドルを返す形に変更
Audio::SoundHandle Audio::LoadAudio(const std::string& filename)
{
    std::wstring filePathW = StringUtility::ConvertString(filename);
    HRESULT result;

    // 1. ソースリーダー作成
    ComPtr<IMFSourceReader> pReader;
    result = MFCreateSourceReaderFromURL(filePathW.c_str(), nullptr, &pReader);
    if (FAILED(result)) {
        assert(false && "Failed to load audio file.");
        return -1; // エラー時は最大値を返す等の処理
    }

    // 2. メディアタイプ設定 (PCM)
    ComPtr<IMFMediaType> pPCMType;
    MFCreateMediaType(&pPCMType);
    pPCMType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    pPCMType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    result = pReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, pPCMType.Get());
    assert(SUCCEEDED(result));

    // 3. フォーマット取得
    ComPtr<IMFMediaType> pOutType;
    pReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, &pOutType);
    WAVEFORMATEX* waveFormat = nullptr;
    MFCreateWaveFormatExFromMFMediaType(pOutType.Get(), &waveFormat, nullptr);

    SoundData soundData = {};
    soundData.wfex = *waveFormat;
    CoTaskMemFree(waveFormat);

    // 4. データ読み込みループ
    while (true)
    {
        ComPtr<IMFSample> pSample;
        DWORD flags = 0;
        result = pReader->ReadSample(
            (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
            0, nullptr, &flags, nullptr, &pSample);

        if (flags & MF_SOURCE_READERF_ENDOFSTREAM) break;

        if (pSample)
        {
            ComPtr<IMFMediaBuffer> pBuffer;
            pSample->ConvertToContiguousBuffer(&pBuffer);
            BYTE* pData = nullptr;
            DWORD currentLength = 0;
            pBuffer->Lock(&pData, nullptr, &currentLength);
            
            soundData.buffer.insert(soundData.buffer.end(), pData, pData + currentLength);
            pBuffer->Unlock();
        }
    }

    // 5. ハンドル発行と登録
    SoundHandle handle = nextHandle_;
    soundDatas_[handle] = soundData;
    
    // 次のハンドル番号を準備
    nextHandle_++;

    return handle;
}

void Audio::UnloadAudio(SoundHandle soundHandle)
{
    // マップから削除（メモリ解放される）
    soundDatas_.erase(soundHandle);
}

void Audio::PlayAudio(SoundHandle soundHandle, bool loop, float volume)
{
    // 指定されたハンドルがマップにあるか確認
    if (soundDatas_.find(soundHandle) == soundDatas_.end()) {
        return; // 無効なハンドルなら何もしない
    }

    // 参照でデータ取得（コピーしない）
    const SoundData& soundData = soundDatas_[soundHandle];

    HRESULT result;
    IXAudio2SourceVoice* pSourceVoice = nullptr;
    result = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
    assert(SUCCEEDED(result));

    pSourceVoice->SetVolume(volume);

    XAUDIO2_BUFFER buf = {};
    buf.AudioBytes = (UINT32)soundData.buffer.size();
    buf.pAudioData = soundData.buffer.data();
    buf.Flags = XAUDIO2_END_OF_STREAM;

    if (loop) {
        buf.LoopCount = XAUDIO2_LOOP_INFINITE;
    }

    result = pSourceVoice->SubmitSourceBuffer(&buf);
    result = pSourceVoice->Start();

  // Voice登録
    Voice voice{};
    voice.handle = nextVoiceHandle_++;
    voice.sourceVoice = pSourceVoice;
    activeVoices_.push_back(voice);
}

void Audio::StopAudio(SoundHandle voiceHandle)
{
    auto it = std::find_if(
        activeVoices_.begin(), activeVoices_.end(),
        [voiceHandle](const Voice& v) { return v.handle == voiceHandle; }
    );

    if (it != activeVoices_.end())
    {
        it->sourceVoice->Stop();
        it->sourceVoice->FlushSourceBuffers();
        it->sourceVoice->DestroyVoice();
        activeVoices_.erase(it);
    }
}

void Audio::PauseAudio(SoundHandle voiceHandle)
{
      auto it = std::find_if(
        activeVoices_.begin(), activeVoices_.end(),
        [voiceHandle](const Voice& v) { return v.handle == voiceHandle; }
    );

    if (it != activeVoices_.end())
    {
        it->sourceVoice->Stop();
    }
}

void Audio::ResumeAudio(SoundHandle voiceHandle)
{
       auto it = std::find_if(
        activeVoices_.begin(), activeVoices_.end(),
        [voiceHandle](const Voice& v) { return v.handle == voiceHandle; }
    );

    if (it != activeVoices_.end())
    {
        it->sourceVoice->Start();
    }
}

bool Audio::IsPlaying(SoundHandle voiceHandle)
{
 auto it = std::find_if(
        activeVoices_.begin(), activeVoices_.end(),
        [voiceHandle](const Voice& v) { return v.handle == voiceHandle; }
    );

    if (it != activeVoices_.end())
    {
        XAUDIO2_VOICE_STATE state{};
        it->sourceVoice->GetState(&state);
        return state.BuffersQueued > 0;
    }

    return false;}
