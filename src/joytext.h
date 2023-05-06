#include <stdio.h>

#define GL_GLEXT_PROTOTYPES
#include <SDL.h>
#include <SDL_opengl.h>

#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include "../lib/stb_truetype.h"

#include "types.h"
#include "math.h"

typedef struct
{
    s32 Count;
    u8 *Data;
} buffer;

typedef enum
{
    result_Ok,
    result_Error,
} result;

typedef struct
{
    b32 Running;
    s32 StartingBaseline;
    s32 CurrentLine;
} state;
