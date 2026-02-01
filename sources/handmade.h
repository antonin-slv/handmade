#ifndef HANDMADE_H
#define HANDMADE_H
#include <string>

struct HandmadeScreenBuffer
{
    void *Memory;
    int Width;
    int Height;
    int Pitch;
};

struct HandmadeSoundOutput
{
    float Volume = 1.0f; // entre 0.0f et 1.0f

    int SampleRate;  // in Hz
    float Frequency; // in Hz

    int SampleIndex = 0;
    int channels = 2;
    int bitsPerSample = 32;
};

static float SinWaveLastPhase = 0.0f;

/*
    Functions that fills frameCount Frames into the audioBuffer
*/
void HandmadeFillAudioBuffer(
    void *audioBuffer,
    HandmadeSoundOutput &soundOutput,
    int frameCount);

void HandmadeUpdateAndRender(HandmadeScreenBuffer *Buffer, int xOffset, int yOffset, float zoom_level);

void renderArrayPattern(HandmadeScreenBuffer *Buffer,
                        int *array, int array_width, int array_height,
                        float cell_size, int x_offset, int y_offset,
                        float zoom_level);
                        
void renderString(HandmadeScreenBuffer *buffer, const std::string &str, int x, int y);

#endif // HANDMADE_H