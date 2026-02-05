#ifndef HANDMADE_H
#define HANDMADE_H
#include <string>
#include "hand_keyboard.h"
#include "Engine/visual_func.h"
#include "os_api.h"


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

void renderArrayPattern(HandmadeScreenBuffer *Buffer, RenderingArray array,
                        int x_offset, int y_offset, float zoom_level);

void renderString(HandmadeScreenBuffer *buffer, const std::string &str, int x, int y);

// in the end the only function ?
static void HandmadeUpdateAndRender(HandmadeScreenBuffer *Buffer, HandmadeSoundOutput *SoundOutput, unified_input InputState, float deltaT, int queriedAudioFrames = 512);

static void HandmadeInitialize();

static void HmadeOnBufferSizeChange(int new_width, int new_height);

#endif // HANDMADE_H