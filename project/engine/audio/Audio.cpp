#include "Audio.h"
#include <cassert>
#include "StringUtility.h"
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>

#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfuuid.lib") 

#include <mfreadwrite.h>
#pragma comment(lib, "mfreadwrite.lib")

using namespace Microsoft::WRL;
Audio::~Audio()
{
    masteringVoice_->DestroyVoice(); // マスターボイスの破棄

    xAudio2_.Reset();
    SoundUnload(&soundData_); // 音声データの解放
    HRESULT result;
    result = MFShutdown();
    assert(SUCCEEDED(result));
}
void Audio::Initialize()
{
    HRESULT result;
    // XAudio2の初期化
    result = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
    assert(SUCCEEDED(result));
    // マスターボイスの作成
    result = xAudio2_->CreateMasteringVoice(&masteringVoice_);
    assert(SUCCEEDED(result));
    // MFの初期化
    result = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
    assert(SUCCEEDED(result));


}

Audio::SoundData Audio::SoundLoadWave(const  std::string& filename)
{
    std::wstring filePathW = StringUtility::ConvertString(filename);
    HRESULT result;
    // MFソースリーダーの作成
   ComPtr<IMFSourceReader> pReader;
    result = MFCreateSourceReaderFromURL(filePathW.c_str(), nullptr, &pReader);
    assert(SUCCEEDED(result));
    // PCM形式での出力を指定   
    ComPtr<IMFMediaType> pPCMType;
    MFCreateMediaType(&pPCMType);
    pPCMType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    pPCMType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    result = pReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, pPCMType.Get());
    assert(SUCCEEDED(result));
    //　セットされたメディアタイプの情報を取得
    ComPtr<IMFMediaType> pOutType;
    pReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, &pOutType);
    // 波形フォーマットの取得
    WAVEFORMATEX* waveFormat = nullptr;
    MFCreateWaveFormatExFromMFMediaType(pOutType.Get(), &waveFormat, nullptr);
    // 音声データの読み込み
    SoundData soundData = {};
    soundData.wfex = *waveFormat;
    CoTaskMemFree(waveFormat); // 波形フォーマット情報の解放
    // PMCデータのバッファ構築
    while (true)
    {
        ComPtr<IMFSample> pSample;
        DWORD streamIndex=0, flags=0;
        LONGLONG llTimeStamp = 0;
        // サンプルの読み込み
        result = pReader->ReadSample(
            (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
            0,
            &streamIndex,
            &flags,
            &llTimeStamp,
            &pSample);
        // ストリームの終端に達した場合はループを抜ける
        if (flags & MF_SOURCE_READERF_ENDOFSTREAM) break;
        if (pSample)
        {
            ComPtr<IMFMediaBuffer> pBuffer;
            // サンプルからメディアバッファを取得
            result = pSample->ConvertToContiguousBuffer(&pBuffer);
            assert(SUCCEEDED(result));
            // バッファのロック
            BYTE* pData = nullptr;
            DWORD maxLength = 0, currentLength = 0;
            result = pBuffer->Lock(&pData, &maxLength, &currentLength);
            assert(SUCCEEDED(result));
            // 音声データをSoundDataのバッファにコピー
            soundData.buffer.insert(soundData.buffer.end(), pData, pData + currentLength);
            pBuffer->Unlock(); // バッファのアンロック
        }


    }
    return soundData;
}

void Audio::PlayAudio(const SoundData& soundData)
{
    SetSoundData(soundData); // 音声データを設定
    SoundPlayWave(xAudio2_.Get(), soundData_); // 音声の再生
}

void Audio::SoundUnload(SoundData* soundData)
{
    soundData->buffer.clear(); // バッファの解放
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
    buf.AudioBytes = (UINT32)SoundData.buffer.size(); // 音声データのサイズ
    buf.pAudioData = SoundData.buffer.data(); // 音声データへのポインタ
    buf.Flags = XAUDIO2_END_OF_STREAM; // ストリームの終端を示すフラグ

    result = pSourceVoice->SubmitSourceBuffer(&buf);
    result = pSourceVoice->Start(); // 音声の再生を開始
}
