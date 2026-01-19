// this file contains definitions and function to get an easier access to win32 audio engine features
#include <mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>
#include <iostream>
#include <string>
#include <Audioclient.h>
#include <Audiopolicy.h>

#define REFTIMES_PER_SEC 10000000
#define REFTIMES_PER_MILLISEC 10000


static IAudioClient* pAudioClient;
static UINT32 bufferFrameCount;

#define SAFE_RELEASE(punk) \
    if ((punk) != NULL)    \
    {                      \
        (punk)->Release(); \
        (punk) = NULL;     \
    }



HRESULT win32_GetRenderClient(IAudioRenderClient **pRenderClient,  WAVEFORMATEX  ** pwfx )
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

    //output Device name for debugging
    IPropertyStore *propertyStore;
    PROPVARIANT friendlyName;
    hr =pDevice->OpenPropertyStore(STGM_READ, &propertyStore);
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

//to be called at the end of the program to release the audio client
void win32_releaseALLAudioClient()
{
    SAFE_RELEASE(pAudioClient)
}


void fillAudioBuffer(
    IAudioClient &audioClient,
    IAudioRenderClient *renderClient,
    WAVEFORMATEX *pwfx,
    DWORD &audioFlags,
    int &wavetime)
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

        WORD BytePerSample = pwfx->wBitsPerSample / 8;
        // Load data into the shared buffer.
        for (UINT32 i = 0; i < availableFrameCount; i++)
        {

          // we just do 800Hz square wave for testing
          bool isPositive = (wavetime++ % (pwfx->nSamplesPerSec / 800) < (pwfx->nSamplesPerSec / 1600));

          for (int channel = 0; channel < pwfx->nChannels; channel++)
          {
            if (pwfx->wBitsPerSample == 32)
            {
              // Format FLOAT
              *((float *)pAudioData) = isPositive ? 0.1f : -0.1f;
            }
            else
            {
              // Format INT16
              *((int16_t *)pAudioData) = isPositive ? 3000 : -3000;
            }
            // On avance du nombre RÉEL d'octets par échantillon
            pAudioData += BytePerSample;
          }
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