#ifndef HANDMADE_H
#define HANDMADE_H
#include <string>
#include "hand_keyboard.h"
#include "Engine/visual_func.h"
#include "os_api.h"


#define KILOBYTES(Value) ((Value)*1024LL)
#define MEGABYTES(Value) (KILOBYTES(Value)*1024LL)
#define GIGABYTES(Value) (MEGABYTES(Value)*1024LL)

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

#if HANDMADE_FAST
#define Assert(Expression) 
#else
#define Assert(Expression) if(!Expression) {*(int * )0 = 0;}
#endif

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


// for 3D rendering, must be allocated once...
static float *depth_buffer = nullptr;

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


// in the end the only function ?
static void HandmadeUpdateAndRender(HandmadeScreenBuffer *Buffer, HandmadeSoundOutput *SoundOutput, unified_input InputState, float deltaT, int queriedAudioFrames = 512);

static void HandmadeInitialize();

static void HmadeOnBufferSizeChange(int new_width, int new_height);

#endif // HANDMADE_H