// this file contains definitions and function to get an easier access to win32 audio engine features
#include <iostream>
#include <string>

#define REFTIMES_PER_SEC 10000000
#define REFTIMES_PER_MILLISEC 10000

enum WaveShape
{
    WAVE_SHAPE_SINE,
    WAVE_SHAPE_SQUARE
};

struct HandmadeSoundOutput
{
    float Volume = 1.0f; // entre 0.0f et 1.0f

    int SampleRate;  // in Hz
    float Frequency; // in Hz

    int SampleIndex = 0;
    WaveShape WaveShape;
    int channels = 2;
    int bitsPerSample = 32;
};

static float SinWaveLastPhase = 0.0f;


void renderSquareWave(float *buffer, HandmadeSoundOutput &soundOutput, int frameCount)
{
    int samampleCount = frameCount * soundOutput.channels;
    for (int i = 0; i < samampleCount; i += 1)
    {
        bool isPositive = (soundOutput.SampleIndex++ % ((soundOutput.SampleRate * 2) / (int)soundOutput.Frequency) < (soundOutput.SampleRate / (int)(soundOutput.Frequency)));
        buffer[i] = isPositive ? soundOutput.Volume : -soundOutput.Volume;
    }
}

void renderSineWave(float *buffer, HandmadeSoundOutput &soundOutput, int frameCount, float &lastPhase)
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


void HandmadeFillAudioBuffer(
    void *audioBuffer,
    HandmadeSoundOutput &soundOutput,
    int frameCount) {

    float *buffer = (float *)audioBuffer;
    int sampleCount = frameCount * soundOutput.channels;

    switch (soundOutput.WaveShape)
    {
    case WAVE_SHAPE_SQUARE:
        renderSquareWave(buffer, soundOutput, frameCount);
        break;
    case WAVE_SHAPE_SINE:
    default:
        renderSineWave(buffer, soundOutput, frameCount, SinWaveLastPhase);
        break;
    }
}