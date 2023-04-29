#include "joytext.h"

#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 500

unsigned char ttf_buffer[1<<20];
unsigned char temp_bitmap[512*512];

stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs
GLuint ftex;

static void my_stbtt_initfont(void)
{
    fread(ttf_buffer, 1, 1<<20, fopen("../fonts/Arial Black.ttf", "rb"));
    stbtt_BakeFontBitmap(ttf_buffer,0, 32.0, temp_bitmap,512,512, 32,96, cdata); // no guarantee this fits!
    // can free ttf_buffer at this point
    glGenTextures(1, &ftex);
    glBindTexture(GL_TEXTURE_2D, ftex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512,512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, temp_bitmap);
    // can free temp_bitmap at this point
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

static void my_stbtt_print(float x, float y, u8 *text)
{
    // assume orthographic projection with units = screen pixels, origin at top left
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, ftex);
    glBegin(GL_QUADS);
    glColor3f(1, 0.1, 0.1);
    while (*text) {
        if (*text >= 32 && *text < 128) {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(cdata, 512,512, *text-32, &x,&y,&q,1);//1=opengl & d3d10+,0=d3d9
            glTexCoord2f(q.s0,q.t0); glVertex2f(q.x0,q.y0);
            glTexCoord2f(q.s1,q.t0); glVertex2f(q.x1,q.y0);
            glTexCoord2f(q.s1,q.t1); glVertex2f(q.x1,q.y1);
            glTexCoord2f(q.s0,q.t1); glVertex2f(q.x0,q.y1);
        }
        ++text;
    }
    glEnd();
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
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
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

static void DeInit(SDL_Window *Window/*, u32 VertexArray, u32 VertexBuffer, u32 ElementBuffer*/)
{
    printf("DeInit\n");
    /* glDeleteVertexArrays(1, &VertexArray); */
    /* glDeleteBuffers(1, &VertexBuffer); */
    /* glDeleteBuffers(1, &ElementBuffer); */
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
            case SDLK_w:
            {
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

static void draw(void)
{
   glViewport(0,0,SCREEN_WIDTH,SCREEN_HEIGHT);
   glClearColor(0.45f,0.45f,0.75f,0);
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
   glColor3f(1,1,1);

   // show font bitmap
   glBegin(GL_QUADS);
      glTexCoord2f(0,0); glVertex2f(256,200+0);
      glTexCoord2f(1,0); glVertex2f(768,200+0);
      glTexCoord2f(1,1); glVertex2f(768,200+512);
      glTexCoord2f(0,1); glVertex2f(256,200+512);
   glEnd();
}

static result TestJoyText(void)
{
    result Result = result_Ok;
    /* GLuint ID; */
    /* u32 VertexBuffer, VertexArray, ElementBuffer, */
    u32 DelayInMilliseconds = 16;
    state State;
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
    my_stbtt_initfont();
    /* ID = glCreateProgram(); */
    /* SetupShaders(ID); */
    /* SetupTextures(ID, &VertexBuffer, &VertexArray, &ElementBuffer, &texture); */
    SDL_ShowWindow(Window);
    while(State.Running)
    {
        HandleEvents(&State);
        /* glClearColor(0.8, 0.79, 0.8, 1.0f); */
        /* glClear(GL_COLOR_BUFFER_BIT); */

        /* UpdateAndRender(ID, &State, VertexArray, VertexBuffer, ElementBuffer); */
        draw();
        /* my_stbtt_print(-0.5, -0.5, (u8 *)"Hello, world"); */
        SDL_GL_SwapWindow(Window);
        SDL_Delay(DelayInMilliseconds); // TODO: figure out when to sleep, only when non-vsync?
    }
    DeInit(Window/*, VertexArray, VertexBuffer, ElementBuffer*/);
    return Result;
}

int main(void)
{
    s32 Result = 0;
    TestJoyText();
    return Result;
}
