#include "joytext.h"

const s32 SCREEN_WIDTH = 800;
const s32 SCREEN_HEIGHT = 700;

#define FONT_HEIGHT_IN_PIXELS 24.0
#define FONT_HEIGHT_IN_PIXELS_INT 24

u8 ttf_buffer[1<<20];
u8 temp_bitmap[512*512];

stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs
GLuint ftex;

static buffer *AllocateBuffer(s32 Count)
{
    s32 CharCount = Count + 1;
    buffer *Result = malloc(sizeof(buffer));
    u8 *Data = malloc(sizeof(u8) * Count);
    memset(Data, 0, Count);
    Result->Count = CharCount;
    Result->Data = Data;
    Result->Data[Count] = 0;
    return Result;
}

static buffer *ReadFileIntoBuffer(char *FilePath)
{
    s32 FileSize;
    FILE *File = fopen(FilePath, "rb");
    buffer *Result = 0;
    if(!File)
    {
        printf("ERROR opening file %s\n", FilePath);
    }
    else
    {
        fseek(File, 0, SEEK_END);
        FileSize = ftell(File);
        Result = AllocateBuffer(FileSize);
        fseek(File, 0, SEEK_SET);
        fread(Result->Data, 1, FileSize, File);
        Result->Data[Result->Count - 1] = 0;
        fclose(File);
    }
    return Result;
}

static void InitFont(void)
{
    fread(ttf_buffer, 1, 1<<20, fopen("../fonts/Arial Black.ttf", "rb"));
    stbtt_BakeFontBitmap(ttf_buffer,0, FONT_HEIGHT_IN_PIXELS, temp_bitmap,512,512, 32,96, cdata); // no guarantee this fits!
    // can free ttf_buffer at this point
    glGenTextures(1, &ftex);
    glBindTexture(GL_TEXTURE_2D, ftex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512,512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, temp_bitmap);
    // can free temp_bitmap at this point
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

static void PrepForDrawingText(void)
{
    glViewport(0,0,SCREEN_WIDTH,SCREEN_HEIGHT);
    glClearColor(0.30f,0.20f,0.30f,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,SCREEN_WIDTH,SCREEN_HEIGHT,0,-1,1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

static void PrintText(float x, float y, u8 text[], s32 Count)
{
    s32 I;
    glBindTexture(GL_TEXTURE_2D, ftex);
    glBegin(GL_QUADS);
    for(I = 0; Count < 0 ? text[I] : I < Count; ++I)
    {
        stbtt_aligned_quad q;
        stbtt_GetBakedQuad(cdata, 512,512, text[I]-32, &x,&y,&q,1);//1=opengl & d3d10+,0=d3d9
        glTexCoord2f(q.s0,q.t0); glVertex2f(q.x0,q.y0);
        glTexCoord2f(q.s1,q.t0); glVertex2f(q.x1,q.y0);
        glTexCoord2f(q.s1,q.t1); glVertex2f(q.x1,q.y1);
        glTexCoord2f(q.s0,q.t1); glVertex2f(q.x0,q.y1);
    }
    glEnd();
}

static s32 GetLineCount(buffer *Buffer)
{
    s32 I, LineCount = 1;
    for(I = 0; I < Buffer->Count; ++I)
    {
        if(Buffer->Data[I] == '\n')
        {
            ++LineCount;
        }
    }
    return LineCount;
}

static void SetLineOffsets(buffer *Buffer, s32 LineOffsets[])
{
    s32 I, CurrentLine = 1;
    LineOffsets[0] = 0; // first line is always zero-offset
    for(I = 0; I < Buffer->Count; ++I)
    {
        if(Buffer->Data[I] == '\n')
        {
            LineOffsets[CurrentLine] = I + 1;
            ++CurrentLine;
        }
    }
}

static void DrawBuffer(state *State, buffer *Buffer, s32 LineOffsets[], s32 LineCount, s32 OffsetY)
{
    s32 I = 0;
    s32 LinesNeeded = (SCREEN_HEIGHT / FONT_HEIGHT_IN_PIXELS_INT) + 8;
    PrepForDrawingText();
    for(I = 0; I < LinesNeeded; ++I)
    {
        s32 LineIndex = State->CurrentLine + I;
        s32 LineOffset = LineOffsets[State->CurrentLine+I];
        if(LineOffset >= 0 && LineOffset < Buffer->Count)
        {
            s32 IsLastLine = LineIndex == LineCount - 1;
            s32 NextOffset = IsLastLine ? Buffer->Count : LineOffsets[LineIndex+1];
            s32 LineLength = NextOffset - LineOffset;
            if(LineOffset + LineLength < Buffer->Count)
            {
                PrintText(0, FONT_HEIGHT_IN_PIXELS_INT*I + OffsetY, &Buffer->Data[LineOffset], LineLength);
            }
        }
    }
}

static result Init()
{
    printf("Init\n");
    result Result = result_Ok;
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        Result = result_Error;
        printf("SDL_Init error");
    }
    return Result;
}

static b32 InitGL()
{
    b32 success = 1;
    GLenum error = GL_NO_ERROR;
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    error = glGetError();
    if(error != GL_NO_ERROR)
    {
        printf("Error initializing OpenGL!\n");
        success = 0;
    }
    error = glGetError();
    if(error != GL_NO_ERROR)
    {
        printf("Error initializing OpenGL!\n");
        success = 0;
    }
    return success;
}

static void DeInit(SDL_Window *Window)
{
    printf("DeInit\n");
    SDL_DestroyWindow(Window);
    SDL_Quit();
}

static void HandleEvents(state *State)
{
    SDL_Event Event;
    while(SDL_PollEvent(&Event))
    {
        switch(Event.type)
        {
        case SDL_KEYDOWN:
        {
            switch(Event.key.keysym.sym)
            {
            case SDLK_DOWN:
            case SDLK_j:
            {
                State->StartingBaseline -= FONT_HEIGHT_IN_PIXELS;
                State->CurrentLine += 1;
            } break;
            case SDLK_UP:
            case SDLK_k:
            {
                State->StartingBaseline += FONT_HEIGHT_IN_PIXELS;
                State->CurrentLine -= 1;
            } break;
            }
        } break;
        case SDL_KEYUP:
        {
            switch(Event.key.keysym.sym)
            {
            case SDLK_w:
            {
            } break;
            }
        } break;
        case SDL_MOUSEBUTTONDOWN:
        {
        } break;
        case SDL_MOUSEBUTTONUP:
        {
        } break;
        case SDL_QUIT:
        {
            State->Running = 0;
        } break;
        }
    }
}

u8 DebugText[1024];
static result TestJoyText(buffer *Buffer, s32 LineOffsets[], s32 LineCount)
{
    result Result = result_Ok;
    u32 DelayInMilliseconds = 16;
    u32 DebugDisplayHeight = 48;
    u64 SleepTime, Now, UpdateTime, Max = 0;
    state State;
    State.Running = 1;
    State.StartingBaseline = 24;
    State.CurrentLine = 0;
    SDL_Window *Window;
    SDL_GLContext GLContext;
    result InitResult = Init();
    if(InitResult == result_Error)
    {
        printf("Init error\n");
        return 1;
    }
    Window = SDL_CreateWindow("joytext", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    assert(Window);
    GLContext = SDL_GL_CreateContext(Window);
    if(GLContext == 0)
    {
        printf("OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
        return 1;
    }
    if(SDL_GL_SetSwapInterval(1) < 0)
    {
        printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
    }
    InitGL();
    InitFont();
    SDL_ShowWindow(Window);
    u32 IterCount = 0;
    while(State.Running)
    {
        Now = SDL_GetTicks64();
        HandleEvents(&State);
        glColor3f(0.80, 0.90, 0.90);
        DrawBuffer(&State, Buffer, LineOffsets, LineCount, DebugDisplayHeight);
        /* PrintBuffer(&State, Buffer, DebugDisplayHeight); */
        glColor3f(0,0,0);
        PrintText(0, 20, DebugText, -1);
        SDL_GL_SwapWindow(Window);
        UpdateTime = SDL_GetTicks64() - Now;
        SleepTime = DelayInMilliseconds - UpdateTime;
        SleepTime = ClampU64(1, SleepTime, DelayInMilliseconds);
        if(IterCount++ > 16)
        {
            Max = MaxU64(Max, UpdateTime);
        }
        /* sprintf((char *)DebugText, "Max Update Time(ms): %llu", Max); */
        sprintf((char *)DebugText, "[Update Time %llums] [Sleep Time: %llu]", UpdateTime, SleepTime);
        /* sprintf((char *)DebugText, "Current Line %d", State.CurrentLine); */
        /* sprintf((char *)DebugText, "Sleep Time(ms): %llu", SleepTime); */
        SDL_Delay(SleepTime);
    }
    DeInit(Window);
    return Result;
}

int main(void)
{
    s32 Result = 0;
    char *FilePath = "../src/joytext.c";
    /* char *FilePath = "../ECMAScript_2022_Language_Specification.html"; */
    buffer *Buffer = ReadFileIntoBuffer(FilePath);
    if(!Buffer)
    {
        printf("ERROR reading file into buffer\n");
    }
    s32 LineCount = GetLineCount(Buffer);
    s32 LineOffsets[LineCount];
    SetLineOffsets(Buffer, LineOffsets);
    /* { s32 I; for(I = 0; I < LineCount; ++I) { if(LineOffsets[I] >= 0 && LineOffsets[I] < Buffer->Count) { printf("%c\n", Buffer->Data[LineOffsets[I]]); } } } */
    TestJoyText(Buffer, LineOffsets, LineCount);
    return Result;
}
