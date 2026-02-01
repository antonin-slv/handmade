#include "handmade.h"

#include "font8x8_basic.h"
#include <stdint.h>
#include <string>

// visual engine functions
void renderCharacter(HandmadeScreenBuffer *buffer, char character, int x, int y)
{
    uint8_t *row = (uint8_t *)buffer->Memory + y * buffer->Pitch + x * 4;
    for (int glyph_y = 0; glyph_y < 8; ++glyph_y)
    {
        uint8_t glyph_row = font8x8_basic[(uint8_t)character][glyph_y];
        uint32_t *pixel = (uint32_t *)row;
        for (int glyph_x = 0; glyph_x < 8; ++glyph_x)
        {
            uint8_t mask = 1 << glyph_x;
            if (glyph_row & mask)
            {
                *pixel++ = (uint32_t)(0x00FFFFFF); // White
            }
            else
            {
                *pixel++ = (uint32_t)(0x00000000); // Black
            }
        }
        row += buffer->Pitch;
    }
}

void renderString(HandmadeScreenBuffer *buffer, const std::string &str, int x, int y)
{
    int cursor_x = x;
    for (char c : str)
    {
        renderCharacter(buffer, c, cursor_x, y);
        cursor_x += 8; // Advance cursor by character width
        if (cursor_x + 8 > buffer->Width)
        {
            break; // Stop rendering if we exceed buffer width
        }
    }
}

void RenderGradient(HandmadeScreenBuffer *Buffer, int XOffset, int YOffset)
{

    int small_white_square_width = 1;
    int small_white_square_height = 50;
    int centerX = Buffer->Width / 2;
    int centerY = Buffer->Height / 2;
    uint8_t *row = (uint8_t *)Buffer->Memory;
    for (int Y = 0; Y < Buffer->Height; ++Y)
    {
        uint32_t *pixel = (uint32_t *)row;
        for (int X = 0; X < Buffer->Width; ++X)
        {

            if ((X - centerX + XOffset) >= 0 && (X - centerX + XOffset) < small_white_square_width &&
                (Y - centerY + YOffset) >= 0 && (Y - centerY + YOffset) < small_white_square_height)
            {
                *pixel++ = 0x00FFFFFF;
            }
            else
            {
                uint8_t blue = (X + XOffset) % 256;
                uint8_t green = (Y + YOffset) % 256;
                uint8_t red = 0;
                *pixel++ = ((red << 16) | (green << 8) | blue);
            }
        }
        row += Buffer->Pitch;
    }
}

void renderArrayPattern(HandmadeScreenBuffer *Buffer,
                        int *array, int array_width, int array_height,
                        float cell_size = 0, int x_offset = 0, int y_offset = 0,
                        float zoom_level = 1.0f)
{

    float StepX, StepY;
    if (cell_size > 0)
    {
        StepX = 1 / ((float)cell_size * zoom_level);
        StepY = 1 / ((float)cell_size * zoom_level);
    }
    else
    {
        StepX = (float)(array_width) / ((float)Buffer->Width * zoom_level);
        StepY = (float)(array_height) / ((float)Buffer->Height * zoom_level);
    }

    uint8_t *row = (uint8_t *)Buffer->Memory;

    for (int Y = 0; Y < Buffer->Height; ++Y)
    {
        uint32_t *pixel = (uint32_t *)row;

        float current_y = (Y + y_offset) * StepY;
        int array_index_y = (int)current_y;
        for (int X = 0; X < Buffer->Width; ++X)
        {
            float current_x = (X + x_offset) * StepX;
            int array_index_x = (int)current_x;

            if (array_index_x >= array_width || array_index_x < 0 || array_index_y >= array_height || array_index_y < 0)
            {
                // out of bounds
                *pixel++ = 0x000F0F0F; // very mid gray
            }
            else
            {
                *pixel++ = array[array_index_y * array_width + array_index_x];
            }
        }
        row += Buffer->Pitch;
    }
}

// audio functions  - - - - - - - - - - -

void renderSineWave(float *buffer, HandmadeSoundOutput &soundOutput, int frameCount, float &lastPhase)
{
    // mono audio :
    //  for (int i = 0; i < frameCount; ++i)
    //  {
    //      float t = (float)soundOutput.SampleIndex++ / (float)soundOutput.SampleRate;
    //      buffer[i] = soundOutput.Volume * sinf(3.14159265f * soundOutput.Frequency * t);
    //  }
    //  stereo audio :
    int sampleCount = frameCount * soundOutput.channels;

    //
    float sinInnerFactor = (soundOutput.Frequency * 3.14159265f * 2.0f) / (float)soundOutput.SampleRate;
    if (lastPhase != sinInnerFactor * (float)soundOutput.SampleIndex)
    {
        int phaseDiff = (int)((lastPhase - sinInnerFactor * (float)soundOutput.SampleIndex) / (sinInnerFactor));
        soundOutput.SampleIndex += phaseDiff;
    }

    for (int i = 0; i < sampleCount; i += soundOutput.channels)
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
    int frameCount)
{

    float *buffer = (float *)audioBuffer;
    int sampleCount = frameCount * soundOutput.channels;

    renderSineWave(buffer, soundOutput, frameCount, SinWaveLastPhase);
}

// generical game functions :

void HandmadeUpdateAndRender(HandmadeScreenBuffer *Buffer, unified_input InputState, float deltaT)
{
    static int xOffset = 0;
    static int yOffset = 0;
    static float zoom_level = 1.0f;     
    mouse_state &MouseState = InputState.Mouse;
    if (MouseState.is_left_down())
    {
      xOffset -= (MouseState.x - MouseState.last_x);
      yOffset -= (MouseState.y - MouseState.last_y);

      SoundStat.Frequency += (float)(MouseState.x - MouseState.last_x) * 100.0f * deltaT;
      if (SoundStat.Frequency < 20.0f)
        SoundStat.Frequency = 20.0f;
      else if (SoundStat.Frequency > 20000.0f)
        SoundStat.Frequency = 20000.0f;
    }
    if (MouseState.wheel_delta != 0)
    {
      float prev_zoom = zoom_level;
      zoom_level *= (1.0f + (float)MouseState.wheel_delta / 4000.0f);

      // pour centrer le zoom : combien de pixels sont ajoutés / retirés ?
      //  si zoom_level augmente, on crop dans l'image, donc on enlève des pixels
      float pixel_change_x = (zoom_level / prev_zoom) * (Buffer->Width / 2 + xOffset) - (Buffer->Width / 2);
      float pixel_change_y = (zoom_level / prev_zoom) * (Buffer->Height / 2 + yOffset) - (Buffer->Height / 2);

      xOffset = (int)pixel_change_x;
      yOffset = (int)pixel_change_y;
    }

    renderArrayPattern(Buffer, test_array, array_width, array_height, 20.0f, xOffset, yOffset, zoom_level);

    float fps = 1.0f / deltaT;
    char fps_buffer[256];
    sprintf_s(fps_buffer, "FPS: %f\tMS: %f", fps, deltaT * 1000.0f);
    renderString(Buffer, fps_buffer, 10, 10);

    char sound_buffer[256];
    sprintf_s(sound_buffer, "Freq: %f Hz", SoundStat.Frequency);
    renderString(Buffer, sound_buffer, 10, 30);

    char keyPressBuffer[256] = {};
    for (int i = 0; i < 256; ++i)
    {
        keyPressBuffer[i] = InputState.Keyboard.keys[i].ended_down ?  toascii(i) : ' ';
    }
    renderString(Buffer, keyPressBuffer, 10, 50);
}

void HandmadeInitialize()
{
    // nothing for now
    array_width = 2000;
    array_height = 2000;
    test_array = (int *)malloc(array_width * array_height * sizeof(int));
    for (int y = 0; y < array_height; y++)
    {
        for (int x = 0; x < array_width; x++)
        {
            uint8_t red = x * 5 % 256;
            uint8_t green = y * 5 % 256;
            uint8_t blue = 0;
            test_array[y * array_width + x] = (red << 16) | (green << 8) | blue;
        }
    }
}

void HandmadeInitializeAudio(int SampleRate)
{
    SoundStat.SampleRate = SampleRate;
    SoundStat.Frequency = 440.0f;
    SoundStat.Volume = 0.5f;
    SoundStat.SampleIndex = 0;
}
