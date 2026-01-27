#include <stdint.h>
#include <string>

#include "font8x8_basic.h"
/**
 * what I need to print stuff on the screen
 *
 */

struct HandmadeScreenBuffer
{
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};

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

void renderCheckerboard(HandmadeScreenBuffer *Buffer, const int SquareSize, const int x_offset, const int y_offset, int red_square_x = 0, int red_square_y = 0)
{

    int squareXindex = 0;
    int squareYindex = 0;
    uint8_t *row = (uint8_t *)Buffer->Memory;
    for (int Y = 0; Y < Buffer->Height; ++Y)
    {
        uint32_t *pixel = (uint32_t *)row;
        for (int X = 0; X < Buffer->Width; ++X)
        {
            int checkerX = (X + x_offset);
            if (checkerX < 0)
                checkerX = (checkerX / SquareSize) - 1;
            else
                checkerX = checkerX / SquareSize;
            int checkerY = (Y + y_offset);
            if (checkerY < 0)
                checkerY = (checkerY / SquareSize) - 1;
            else
                checkerY = checkerY / SquareSize;

            if (checkerX == red_square_x && checkerY == red_square_y)
            {
                *pixel++ = 0x00FF0000; // Red
                continue;
            }
            else if (checkerX == -1 && checkerY == -1)
            {
                *pixel++ = 0x0000FF00; // Green
                continue;
            }
            else if (checkerX == -1 && checkerY == 0)
            {
                *pixel++ = 0x000000FF; // Blue
                continue;
            }
            else if (checkerX == 0 && checkerY == -1)
            {
                *pixel++ = 0x00FFFF00; // Yellow
                continue;
            }
            if ((checkerX + checkerY) % 2 == 0)
            {
                *pixel++ = 0x00FFFFFF; // White
            }
            else
            {
                *pixel++ = 0x00000000; // Black
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