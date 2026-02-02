#ifndef HANDMADE_H
#define HANDMADE_H
#include <string>
#include "hand_keyboard.h"

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

/** to be called on HandmadeScreenBuffer */
#define colorPixel(Buffer, x, y, color)                                                                \
    if ((x) >= 0 && (x) < (Buffer)->Width && (y) >= 0 && (y) < (Buffer)->Height)                       \
    {                                                                                                  \
        uint32_t *pixel = (uint32_t *)((uint8_t *)(Buffer)->Memory + (y) * (Buffer)->Pitch + (x) * 4); \
        *pixel = (color);                                                                              \
    }

struct HandmadeScreenBuffer
{
    void *Memory;
    int Width;
    int Height;
    int Pitch;
};

struct HandmadeSoundOutput
{
    float *Buffer;

    float Volume = 1.0f; // entre 0.0f et 1.0f

    int SampleRate;  // in Hz
    float Frequency; // in Hz

    int SampleIndex = 0;
    int channels = 2;

    int framesWritten = 0;
};

struct RenderingArray
{
    int *Array;
    int Width;
    int Height;
    int CellSize;
};

static float SinWaveLastPhase = 0.0f;

// temp array
static RenderingArray test_array;
/*
    Functions that fills frameCount Frames into the audioBuffer
    -> should be called regularly to keep the audio buffer filled I guess
*/
void HandmadeFillAudioBuffer(
    HandmadeSoundOutput &soundOutput,
    int frameCount);

void renderArrayPattern(HandmadeScreenBuffer *Buffer, RenderingArray array,
                        int x_offset, int y_offset, float zoom_level);

void renderString(HandmadeScreenBuffer *buffer, const std::string &str, int x, int y);

// in the end the only function ?
static void HandmadeUpdateAndRender(HandmadeScreenBuffer *Buffer, HandmadeSoundOutput *SoundOutput, unified_input InputState, float deltaT, int queriedAudioFrames = 512);

static void HandmadeInitialize();

#endif // HANDMADE_H