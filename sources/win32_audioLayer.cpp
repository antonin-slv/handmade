#include <mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>

#include <Audioclient.h>
#include <Audiopolicy.h>
#include <stdint.h>
#include "handmade.h"

static IAudioClient *pAudioClient;
static uint32_t bufferFrameCount;

#define REFTIMES_PER_SEC 10000000

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

void Win32FillAudioBuffer(
    IAudioClient &audioClient,
    IAudioRenderClient *renderClient,
    DWORD &audioFlags,
    HandmadeSoundOutput &soundOutput)
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

                HandmadeFillAudioBuffer((void *)pAudioData, soundOutput, availableFrameCount);

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

void win32FillMinimumAudioBuffer(
    IAudioClient &audioClient,
    IAudioRenderClient *renderClient,
    DWORD &audioFlags,
    HandmadeSoundOutput &soundOutput,
    float lastFrameDuration)
{
    BYTE *pAudioData;

    UINT32 padding = 0;
    HRESULT hr = audioClient.GetCurrentPadding(&padding);

    if (SUCCEEDED(hr))
    {
        UINT32 availableFrameCount = bufferFrameCount - padding;

        // calculate time remaining in buffer
        float currentBufferedDurationSec = (float)padding / (float)soundOutput.SampleRate;

        // we make sure the buffer fills up to 2 time the last frame duration

        float targetBufferedDurationSec = lastFrameDuration * 10.0f;
        float timeToFillSec = targetBufferedDurationSec - currentBufferedDurationSec;
        if (timeToFillSec < 0.0f)
            timeToFillSec = 0.0f;

        UINT32 framesToWrite = (UINT32)(timeToFillSec * (float)soundOutput.SampleRate);
        if (framesToWrite > availableFrameCount)
            framesToWrite = availableFrameCount;

        if (framesToWrite > 0)
        {
            hr = renderClient->GetBuffer(framesToWrite, &pAudioData);
            if (SUCCEEDED(hr))
            {

                HandmadeFillAudioBuffer((void *)pAudioData, soundOutput, framesToWrite);

                hr = renderClient->ReleaseBuffer(framesToWrite, audioFlags);
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