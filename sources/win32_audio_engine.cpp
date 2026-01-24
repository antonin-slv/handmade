// this file contains definitions and function to get an easier access to win32 audio engine features
#include <mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>
#include <iostream>
#include <string>
#include <Audioclient.h>
#include <Audiopolicy.h>

#define REFTIMES_PER_SEC 10000000
#define REFTIMES_PER_MILLISEC 10000

enum WaveShape
{
    WAVE_SHAPE_SINE,
    WAVE_SHAPE_SQUARE
};

struct Win32SoundOutput
{
    float Volume = 1.0f; // entre 0.0f et 1.0f

    int SampleRate;  // in Hz
    float Frequency; // in Hz

    int SampleIndex = 0;
    WaveShape WaveShape;
    int channels = 2;
};

static IAudioClient *pAudioClient;
static UINT32 bufferFrameCount;
static float SinWaveLastPhase = 0.0f;

#define SAFE_RELEASE(punk) \
    if ((punk) != NULL)    \
    {                      \
        (punk)->Release(); \
        (punk) = NULL;     \
    }

HRESULT win32_GetAudioRenderClient(IAudioRenderClient **pRenderClient, WAVEFORMATEX **pwfx)
{
    IMMDeviceEnumerator *pEnumerator = NULL;
    IMMDevice *pDevice = NULL;
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;

    HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), NULL,
        CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
        (void **)&pEnumerator);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = pEnumerator->GetDefaultAudioEndpoint(
        eRender, eConsole, &pDevice);
    if (FAILED(hr))
    {
        SAFE_RELEASE(pEnumerator)
        return hr;
    }

    // output Device name for debugging
    IPropertyStore *propertyStore;
    PROPVARIANT friendlyName;
    hr = pDevice->OpenPropertyStore(STGM_READ, &propertyStore);
    propertyStore->GetValue(PKEY_Device_FriendlyName, &friendlyName);

    OutputDebugStringW(friendlyName.pwszVal);
    OutputDebugStringA("\n");

    PropVariantClear(&friendlyName);

    hr = pDevice->Activate(
        __uuidof(IAudioClient), CLSCTX_ALL,
        NULL, (void **)&pAudioClient);
    if (FAILED(hr))
    {
        SAFE_RELEASE(pEnumerator)
        SAFE_RELEASE(pDevice)
        return hr;
    }

    hr = pAudioClient->GetMixFormat(pwfx);
    if (FAILED(hr))
    {
        SAFE_RELEASE(pEnumerator)
        SAFE_RELEASE(pDevice)
        SAFE_RELEASE(pAudioClient)
        return hr;
    }

    hr = pAudioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        0,
        hnsRequestedDuration,
        0,
        *pwfx,
        NULL);
    if (FAILED(hr))
    {
        CoTaskMemFree(pwfx);
        SAFE_RELEASE(pEnumerator)
        SAFE_RELEASE(pDevice)
        SAFE_RELEASE(pAudioClient)
        return hr;
    }

    // Get the actual size of the allocated buffer.
    hr = pAudioClient->GetBufferSize(&bufferFrameCount);
    if (FAILED(hr))
    {
        CoTaskMemFree(pwfx);
        SAFE_RELEASE(pEnumerator)
        SAFE_RELEASE(pDevice)
        SAFE_RELEASE(pAudioClient)
        return hr;
    }

    hr = pAudioClient->GetService(
        __uuidof(IAudioRenderClient),
        (void **)pRenderClient);
    if (FAILED(hr))
    {
        CoTaskMemFree(pwfx);
        SAFE_RELEASE(pEnumerator)
        SAFE_RELEASE(pDevice)
        SAFE_RELEASE(pAudioClient)
        return hr;
    }
    return hr;
}

// to be called at the end of the program to release the audio client
void win32_releaseALLAudioClient()
{
    SAFE_RELEASE(pAudioClient)
}

void renderSquareWave(float *buffer, Win32SoundOutput &soundOutput, int frameCount)
{
    int samampleCount = frameCount * soundOutput.channels;
    for (int i = 0; i < samampleCount; i += 1)
    {
        bool isPositive = (soundOutput.SampleIndex++ % ((soundOutput.SampleRate * 2) / (int)soundOutput.Frequency) < (soundOutput.SampleRate / (int)(soundOutput.Frequency)));
        buffer[i] = isPositive ? soundOutput.Volume : -soundOutput.Volume;
    }
}

void renderSineWave(float *buffer, Win32SoundOutput &soundOutput, int frameCount, float &lastPhase)
{
    // mono audio :
    //  for (int i = 0; i < frameCount; ++i)
    //  {
    //      float t = (float)soundOutput.SampleIndex++ / (float)soundOutput.SampleRate;
    //      buffer[i] = soundOutput.Volume * sinf(3.14159265f * soundOutput.Frequency * t);
    //  }
    //  stereo audio :
    int samampleCount = frameCount * soundOutput.channels;

    //
    float sinInnerFactor = (soundOutput.Frequency * 3.14159265f * 2.0f) / (float)soundOutput.SampleRate;
    if (lastPhase != sinInnerFactor * (float)soundOutput.SampleIndex)
    {
        int phaseDiff = (int)((lastPhase - sinInnerFactor * (float)soundOutput.SampleIndex) / (sinInnerFactor));
        soundOutput.SampleIndex += phaseDiff;
    }

    for (int i = 0; i < samampleCount; i += soundOutput.channels)
    {
        for (int ch = 0; ch < soundOutput.channels; ++ch)
        {
            buffer[i + ch] = soundOutput.Volume * sinf(sinInnerFactor * (float)soundOutput.SampleIndex); // different frequency per channel
        }
        soundOutput.SampleIndex++;
    }

    lastPhase = sinInnerFactor * (float)soundOutput.SampleIndex;
}

void Win32FillAudioBuffer(
    IAudioClient &audioClient,
    IAudioRenderClient *renderClient,
    DWORD &audioFlags,
    Win32SoundOutput &soundOutput)
{
    BYTE *pAudioData;

    UINT32 padding = 0;
    HRESULT hr = audioClient.GetCurrentPadding(&padding);

    if (SUCCEEDED(hr))
    {
        UINT32 availableFrameCount = bufferFrameCount - padding;

        if (availableFrameCount > 0)
        {
            hr = renderClient->GetBuffer(availableFrameCount, &pAudioData);
            if (SUCCEEDED(hr))
            {
                switch (soundOutput.WaveShape)
                {
                case WAVE_SHAPE_SQUARE:
                    renderSquareWave((float *)pAudioData, soundOutput, availableFrameCount);
                    break;
                case WAVE_SHAPE_SINE:
                default:
                    renderSineWave((float *)pAudioData, soundOutput, availableFrameCount, SinWaveLastPhase);
                    break;
                }
                hr = renderClient->ReleaseBuffer(availableFrameCount, audioFlags);
                if (FAILED(hr))
                {
                    OutputDebugStringA("Failed to release audio buffer.\n");
                }
            }
            else
            {
                OutputDebugStringA("Failed to get audio buffer.\n");
            }
        }
    }
    else
    {
        OutputDebugStringA("Failed to get current padding.\n");
    }
}