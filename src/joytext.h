#include <stdio.h>

#define GL_GLEXT_PROTOTYPES
#include <SDL.h>
#include <SDL_opengl.h>

#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include "../lib/stb_truetype.h"

typedef int32_t s32;

typedef uint32_t u32;
typedef uint8_t u8;

typedef uint32_t b32;

typedef enum
{
    result_Ok,
    result_Error,
} result;

typedef struct
{
    b32 Running;
} state;
