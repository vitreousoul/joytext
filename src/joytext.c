#include "joytext.h"

#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 700

#define FONT_HEIGHT_IN_PIXELS 24.0
#define FONT_HEIGHT_IN_PIXELS_INT 24

u8 ttf_buffer[1<<20];
u8 temp_bitmap[512*512];

stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs
GLuint ftex;

static buffer *AllocateBuffer(s32 Count)
{
    s32 CharCount = Count + 1;
    size AllocationSize = sizeof(buffer) + (CharCount * sizeof(u8));
    buffer *Result = malloc(AllocationSize);
    Result->Count = CharCount;
    Result->Data = (u8 *)(Result + sizeof(buffer));
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

char DEBUGTEXT[64];
static void PrintBuffer(state *State, buffer *Buffer, s32 OffsetY)
{
    u8 *Text = Buffer->Data;
    s32 I;
    s32 LineNumber = 1, Count = 0, Baseline = State->StartingBaseline;
    memset(DEBUGTEXT, 0, 64);
    PrepForDrawingText();
    for(I = 0; I < Buffer->Count; ++I)
    {
        u8 Char = Buffer->Data[I];
        if(Char == '\n')
        {
            b32 LineVisible = Baseline > -FONT_HEIGHT_IN_PIXELS_INT && Baseline < SCREEN_HEIGHT;
            if(LineVisible)
            {
                sprintf(DEBUGTEXT, "%d", LineNumber);
                PrintText(0, Baseline + OffsetY, (u8 *)DEBUGTEXT, 64);
                PrintText(64, Baseline + OffsetY, Text, Count);
            }
            ++LineNumber;
            Text = &Buffer->Data[I] + 1;
            Count = 0;
            Baseline += FONT_HEIGHT_IN_PIXELS_INT;
        }
        ++Count;
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
            } break;
            case SDLK_UP:
            case SDLK_k:
            {
                State->StartingBaseline += FONT_HEIGHT_IN_PIXELS;
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
static result TestJoyText(buffer *Buffer)
{
    result Result = result_Ok;
    u32 DelayInMilliseconds = 16;
    u32 DebugDisplayHeight = 48;
    u64 SleepTime, Now, UpdateTime, Max = 0;
    state State;
    State.Running = 1;
    State.StartingBaseline = 24;
    SDL_Window *Window;
    SDL_GLContext GLContext;
    result InitResult = Init();
    if(InitResult == result_Error)
    {
        printf("Init error");
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
        PrintBuffer(&State, Buffer, DebugDisplayHeight);
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
        sprintf((char *)DebugText, "Update Time %llums", UpdateTime);
        /* sprintf((char *)DebugText, "Sleep Time(ms): %llu", SleepTime); */
        SDL_Delay(SleepTime);
    }
    DeInit(Window);
    return Result;
}

int main(void)
{
    s32 Result = 0;
    buffer *Buffer = ReadFileIntoBuffer("../src/joytext.c");
    if(!Buffer)
    {
        printf("ERROR reading file into buffer\n");
    }
    TestJoyText(Buffer);
    return Result;
}
