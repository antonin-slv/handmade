#ifndef VISUAL_FUNC_H
#define VISUAL_FUNC_H

#include "../Math/Shape3D.h"
#include "../hand_keyboard.h"

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

#define COLOR_PIXEL(Buffer, x, y, color)                                                        \
    {                                                                                                  \
        uint32_t *pixel = (uint32_t *)((uint8_t *)(Buffer)->Memory + (y) * (Buffer)->Pitch + (x) * 4); \
        *pixel = (color);                                                                              \
    }
/** to be called on HandmadeScreenBuffer */
#define SAFE_COLOR_PIXEL(Buffer, x, y, color)                                              \
    {                                                                                \
        if ((x) >= 0 && (x) < (Buffer)->Width && (y) >= 0 && (y) < (Buffer)->Height) \
            COLOR_PIXEL(Buffer, x, y, color)                                  \
    }
struct HandmadeScreenBuffer
{
    void *Memory;
    int Width;
    int Height;
    int Pitch;
};

struct RenderingArray
{
    int *Array;
    int Width;
    int Height;
    int CellSize;
};



#endif // VISUAL_FUNC_H