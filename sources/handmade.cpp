#include "handmade.h"

#include "Engine/visual_func.cpp"

#include "font8x8_basic.h"
#include "Math/Shape3D.cpp"
#include "Math/Point3D.h"
#include <stdint.h>
#include <string>

static WireFrame3D cube_shape = {};
static Mesh3D cube_mesh = {};

// audio functions  - - - - - - - - - - -

void renderSineWave(HandmadeSoundOutput &soundOutput, int frameCount, float &lastPhase)
{
    int sampleCount = frameCount * soundOutput.channels;
    //
    float sinInnerFactor = (soundOutput.Frequency * 3.14159265f * 2.0f) / (float)soundOutput.SampleRate;
    float phaseDiff = 0.0f;
    if (lastPhase != sinInnerFactor * (float)soundOutput.SampleIndex)
    {
        phaseDiff = ((lastPhase - sinInnerFactor * (float)soundOutput.SampleIndex) / (sinInnerFactor));
    }

    for (int i = 0; i < sampleCount; i += soundOutput.channels)
    {
        for (int ch = 0; ch < soundOutput.channels; ++ch)
        {
            soundOutput.Buffer[i + ch] = soundOutput.Volume * sinf(sinInnerFactor * (soundOutput.SampleIndex + phaseDiff)); // different frequency per channel
        }
        soundOutput.SampleIndex++;
    }

    lastPhase = sinInnerFactor * (soundOutput.SampleIndex + phaseDiff);
}

void HandmadeFillAudioBuffer(
    HandmadeSoundOutput &soundOutput,
    int frameCount)
{
    if (frameCount <= 0)
        return;
    soundOutput.framesWritten = frameCount;
    renderSineWave(soundOutput, frameCount, SinWaveLastPhase);
}

void WriteMouseToArray(mouse_state &MouseState, HandmadeScreenBuffer *Buffer,
                       int xOffset, int yOffset, float zoom_level)
{

    int real_x = MouseState.x + xOffset;
    int real_y = MouseState.y + yOffset;
    float StepX, StepY;
    if (test_array.CellSize > 0)
    {
        StepX = 1 / ((float)test_array.CellSize * zoom_level);
        StepY = 1 / ((float)test_array.CellSize * zoom_level);
    }
    else
    {
        StepX = (float)(test_array.Width) / ((float)Buffer->Width * zoom_level);
        StepY = (float)(test_array.Height) / ((float)Buffer->Height * zoom_level);
    }

    int array_x = (int)(real_x * StepX);
    int array_y = (int)(real_y * StepY);

    int old_array_x = (int)((MouseState.last_x + xOffset) * StepX);
    int old_array_y = (int)((MouseState.last_y + yOffset) * StepY);

    if (array_x == old_array_x && array_y == old_array_y)
    {
        // do nothing
    }
    else if (abs(array_x - old_array_x) >= abs(array_y - old_array_y))
    {
        // iterate over x
        int step = (array_x > old_array_x) ? 1 : -1;
        for (int x_iter = old_array_x + step; x_iter != array_x; x_iter += step)
        {
            int y_iter = old_array_y + (array_y - old_array_y) * (x_iter - old_array_x) / (array_x - old_array_x);
            if (y_iter >= 0 && y_iter < test_array.Height && x_iter >= 0 && x_iter < test_array.Width)
            {
                int index = y_iter * test_array.Width + x_iter;
                test_array.Array[index] = 0x00FFFFFF; // white
            }
        }
    }
    else
    {
        // iterate over y
        int step = (array_y > old_array_y) ? 1 : -1;
        for (int y_iter = old_array_y + step; y_iter != array_y; y_iter += step)
        {
            int x_iter = old_array_x + (array_x - old_array_x) * (y_iter - old_array_y) / (array_y - old_array_y);
            if (y_iter >= 0 && y_iter < test_array.Height && x_iter >= 0 && x_iter < test_array.Width)
            {
                int index = y_iter * test_array.Width + x_iter;
                test_array.Array[index] = 0x00FFFFFF; // white
            }
        }
    }

    if (array_x >= 0 && array_x < test_array.Width && array_y >= 0 && array_y < test_array.Height)
    {
        int index = array_y * test_array.Width + array_x;
        test_array.Array[index] = 0x00000000; // black

        /* This code writes a black circle to the Array
        // we color all pixels in a circle of fixes radius on screen space
        //first, how much pixels in array space is that ?

        int radius_in_pixels = 5;
        int radius_in_array_x = (int)(radius_in_pixels * StepX);
        int radius_in_array_y = (int)(radius_in_pixels * StepY);

        int array_radius = radius_in_array_x * radius_in_array_y;

        for (int dy = -radius_in_array_y; dy <= radius_in_array_y; ++dy)
        {
            for (int dx = -radius_in_array_x; dx <= radius_in_array_x; ++dx)
            {
                if (dx * dx + dy * dy <= array_radius)
                {
                    int draw_x = array_x + dx;
                    int draw_y = array_y + dy;
                    if (draw_x >= 0 && draw_x < test_array.Width && draw_y >= 0 && draw_y < test_array.Height)
                    {
                        int index = draw_y * test_array.Width + draw_x;
                        test_array.Array[index] = 0x00000000; // black
                    }
                }
            }
        }

        */
    }
}

/** draws a circle arround the mouse position
 *  directly on the screenBuffer to visualize the mouse position
 *
 */
void DrawMouseCircle(HandmadeScreenBuffer *Buffer, mouse_state &MouseState, int radius = 5)
{

    // Mouse.x² + Mouse.y² = r²
    // perimeter = 2 * pi * r
    // we have to draw 2pi*r points
    int perimeter = (int)(2.0f * 3.14159265f * radius);
    for (int i = 0; i < perimeter; ++i)
    {
        float angle = i / (float)radius;
        int draw_x = MouseState.x + int(radius * cosf(angle));
        int draw_y = MouseState.y + int(radius * sinf(angle));
        SAFE_COLOR_PIXEL(Buffer, draw_x, draw_y, 0x00A0F0F0)
    }
}

void DrawSoundBufferVisualization(HandmadeScreenBuffer *Buffer, HandmadeSoundOutput *soundOutput)
{
    // draw a simple sine wave visualization at the bottom of the screen
    int centerY = Buffer->Height - 100;
    int amplitude = 50;
    int length = Buffer->Width;

    // we iterate through the sound buffer instead
    int samplesToDraw = soundOutput->framesWritten * soundOutput->channels;
    for (int i = 0; i < samplesToDraw; i += soundOutput->channels)
    {
        float sample = soundOutput->Buffer[i]; // we take only the first channel for visualization
        int x = (i / soundOutput->channels) % length;
        int y = centerY + (int)(sample * amplitude);
        SAFE_COLOR_PIXEL(Buffer, x, y, 0x00FF00FF)
    }
}

// generical game functions :

void HandmadeUpdateAndRender(HandmadeScreenBuffer *Buffer, HandmadeSoundOutput *SoundStat,
                             unified_input InputState, float deltaT, int queriedAudioFrames)
{

    for (int i = 0; i < Buffer->Width * Buffer->Height; ++i)
    {
        depth_buffer[i] = FLT_MAX;
    }

    static int xOffset = 0;
    static int yOffset = 0;
    static float zoom_level = 1.0f;
    mouse_state &MouseState = InputState.Mouse;

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
    if (InputState.Keyboard.was_key_pressed('R'))
    {
        xOffset = 0;
        yOffset = 0;
        zoom_level = 1.0f;
    }

    if (InputState.Keyboard.is_ctrl_down())
    {
        if (MouseState.left.ended_down)
        {
            WriteMouseToArray(MouseState, Buffer, xOffset, yOffset, zoom_level);
        }
    }
    else if (MouseState.left.ended_down)
    {
        xOffset -= (MouseState.x - MouseState.last_x);
        yOffset -= (MouseState.y - MouseState.last_y);

        SoundStat->Frequency += (float)(MouseState.x - MouseState.last_x) * 100.0f * deltaT;
        if (SoundStat->Frequency < 20.0f)
            SoundStat->Frequency = 20.0f;
        else if (SoundStat->Frequency > 20000.0f)
            SoundStat->Frequency = 20000.0f;
    }
    InputState.Keyboard.keys[Key_Ctrl].ended_down = false;
    InputState.Keyboard.keys[Key_Ctrl].half_transition_count = 0;
    InputState.Keyboard.keys[Key_Ctrl].started_down = false;
    renderArrayPattern(Buffer, test_array, xOffset, yOffset, zoom_level);

    float fps = 1.0f / deltaT;
    char fps_buffer[256];
    sprintf_s(fps_buffer, "FPS: %f\tMS: %f", fps, deltaT * 1000.0f);
    renderString(Buffer, fps_buffer, 10, 10);

    char sound_buffer[256];
    sprintf_s(sound_buffer, "Freq: %f Hz", SoundStat->Frequency);
    renderString(Buffer, sound_buffer, 10, 30);
    char keyPressBuffer[256] = {};
    for (int i = 0; i < 256; ++i)
    {
        keyPressBuffer[i] = InputState.Keyboard.keys[i].ended_down ? toascii(i) : ' ';
    }
    renderString(Buffer, keyPressBuffer, 10, 50);

    DrawMouseCircle(Buffer, MouseState, 10);

    HandmadeFillAudioBuffer(*SoundStat, queriedAudioFrames);

    DrawSoundBufferVisualization(Buffer, SoundStat);

    cube_shape.rotate_degree(Point3D{.5f, 1.0f, 0.0f}, 50.0f * deltaT); // rotate 15 degrees per second around Y axis
    cube_shape.translate(Point3D{0.0f, 0.0f, 5.0f * deltaT});
    // RenderCube(Buffer, cube_shape);
    RenderCubeSides(Buffer, cube_shape);

    cube_mesh.rotate_degree(Point3D{1.0f, 0.5f, 0.0f}, 30.0f * deltaT); // rotate 15 degrees per second around Y axis
    cube_mesh.translate(Point3D{0.0f, 0.0f, 5.0f * deltaT});
    RenderMesh3DWithFaceOrientation(Buffer, cube_mesh, depth_buffer);

    //no vertices drawn yet
    Sphere test_sphere = {};
    test_sphere.center = Point3D{0.0f, 0.0f, 400.0f};
    test_sphere.radius = 100.0f;

    renderSphere3D(Buffer, test_sphere, depth_buffer);

}

void HandmadeInitialize()
{
    // nothing for now
    test_array.Width = 2000;
    test_array.Height = 2000;
    test_array.CellSize = 10;
    test_array.Array = (int *)malloc(test_array.Width * test_array.Height * sizeof(int));
    for (int y = 0; y < test_array.Height; y++)
    {
        for (int x = 0; x < test_array.Width; x++)
        {
            uint8_t red = x * 5 % 256;
            uint8_t green = y * 5 % 256;
            uint8_t blue = 0;
            test_array.Array[y * test_array.Width + x] = (red << 16) | (green << 8) | blue;
        }
    }

    cube_shape = GetSimpleCube();

    cube_shape.scale(120.0f);                           // make the cube bigger
    cube_shape.translate(Point3D{500.0f, 500.0f, 200}); // move away from camera

    cube_mesh = GetCubeMesh();
    cube_mesh.scale(120.0f);          // make the cube bigger
    cube_mesh.translate({0, 0, 400}); // move away from camera
}

void HmadeOnBufferSizeChange(int new_width, int new_height)
{
    if (depth_buffer)
    { // marks the old buffer for deallocation
        free(depth_buffer);
    }
    depth_buffer = (float *)realloc(depth_buffer, sizeof(float) * new_width * new_height);
}
