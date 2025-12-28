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
    xAudio2_.Reset();
    SoundUnload(&soundData_); // 音声データの解放
}
void Audio::Initialize()
{
    HRESULT result;
    // 
    result = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
    assert(SUCCEEDED(result));

      // XAudio2エンジンの作成
    result = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
    assert(SUCCEEDED(result));
    //// マスターボイスの作成
    result = xAudio2_->CreateMasteringVoice(&masteringVoice_);
    assert(SUCCEEDED(result));
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
    //メディアタイプの取得
    ComPtr<IMFMediaType> pOutType;
    result = pReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, &pOutType);

    //波形フォーマットの取得
    WAVEFORMATEX* waveFormat = nullptr;
    MFCreateWaveFormatExFromMFMediaType(pOutType.Get(), &waveFormat, NULL);

    //音声データの読み込み
    SoundData soundData = {};
    soundData.wfex = *waveFormat; // 波形フォーマットの設定
    CoTaskMemFree(waveFormat); // WAVEFORMATEXの解放

    //サンプルの読み込みループ

    while (true)
    {
        ComPtr<IMFSample> pSample;
        DWORD streamIndex = 0, flags = 0;
        LONGLONG llTimestamp;
        //サンプルの読み込み
        result = pReader->ReadSample(
            MF_SOURCE_READER_FIRST_AUDIO_STREAM,
            0,
            &streamIndex,
            &flags,
            &llTimestamp,
            &pSample
        );
        if (flags & MF_SOURCE_READERF_ENDOFSTREAM) break; // ファイルの終端に達したらループを抜ける

        if (pSample)
        {
            //サンプルからバッファーを取得
            ComPtr<IMFMediaBuffer> pBuffer;
            result = pSample->ConvertToContiguousBuffer(&pBuffer);
            assert(SUCCEEDED(result));
            //バッファーのロック
            BYTE* pData = nullptr;
            DWORD maxLength = 0,currentLength = 0;
            result = pBuffer->Lock(&pData, &maxLength, &currentLength);
            assert(SUCCEEDED(result));
            //音声データをSoundDataにコピー
            soundData.buffer.insert(
                soundData.buffer.end(),
                pData,
                pData + currentLength
            );

            //バッファーのアンロック
            result = pBuffer->Unlock();
            assert(SUCCEEDED(result));
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
    soundData->buffer.clear(); // 音声データバッファを解放
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
    buf.pAudioData = SoundData.buffer.data(); // 音声データバッファ
    buf.AudioBytes = (UINT32)SoundData.buffer.size(); // 音声データサイズ
    buf.Flags = XAUDIO2_END_OF_STREAM; // ストリームの終端を示すフラグ

    result = pSourceVoice->SubmitSourceBuffer(&buf);
    result = pSourceVoice->Start(); // 音声の再生を開始
}
