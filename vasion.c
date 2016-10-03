// The MIT License (MIT)

// Copyright (c) 2016 Theodore Dobyns

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL.h>

////////////////////////////////////////////////////////////////////////////////
// Primitive typedefs
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef unsigned int uint;

typedef float float32;
typedef double float64;

typedef uint8 byte;
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Constants
static const int cScreenWidth = 240;
static const int cScreenHeight = 160;

#define INVADER_ROWS 5
#define INVADER_COLS 11
#define INVADER_BOUNDARY_LEFT 10
#define INVADER_BOUNDARY_RIGHT (cScreenWidth - INVADER_BOUNDARY_LEFT)
#define INVADER_MOVE_QUEUE_SIZE 3

#define MAX_SHIELDS 4
#define MAX_INVADERS (INVADER_ROWS * INVADER_COLS)
#define MAX_TANK_BULLETS 1
#define MAX_INVADER_BULLETS 2
#define MAX_BULLETS 32
#define MAX_TEXTURES 32

#define KEY_LEFT SDL_SCANCODE_LEFT
#define KEY_RIGHT SDL_SCANCODE_RIGHT
#define KEY_FIRE SDL_SCANCODE_Z

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Structures

//-----------------------------------
// Math
typedef struct range {
    float32 min;
    float32 max;
} Range;

typedef struct point {
    float32 x, y;
} Point;

typedef struct ipoint {
    int32 x, y;
} IPoint;

typedef struct rect {
    Point position;
    float32 width;
    float32 height;
} Rect;

typedef struct bounds {
    float32 left, right, top, bottom;
} Bounds;

typedef struct ibounds {
    int32 left, right, top, bottom;
} IBounds;

typedef struct px_collision_data {
    int32 pixelA, pixelB;
} PxCollisionData;

typedef struct paletteTexture {
    SDL_Texture* texture;
    uint8* data;
    int width;
    int height;
} PaletteTexture;
//-----------------------------------

//-----------------------------------
// Configuration
typedef struct config {
    float32 tankSpeed;
    float32 tankBulletSpeed;
    Point tankFireOffset;
    Range invaderMoveDelay;
    Range invaderRowDelay;
    Range invaderFireDelay;
    float32 invaderMoveAmount;
    float32 invaderBulletSpeed;
    float32 invaderDeathTime;
} Config;

Config g_config;
//-----------------------------------

//-----------------------------------
// Bullets
typedef struct bullet_state {
    Rect target;
    bool active;
    int baseTexture;
    int direction;
    int frame;
    int frameCount;
    int* ownerHandle;
} BulletState;
//-----------------------------------

//-----------------------------------
// Shields
typedef struct shield_state {
    Rect target;
    PaletteTexture texture;
} ShieldState;
//-----------------------------------

//-----------------------------------
// Tank
typedef enum tank_mode {
    TankMode_Active,
    TankMode_Dead,
} TankMode;

typedef struct tank_state {
    Rect target;
    TankMode mode;
    int bullets[MAX_TANK_BULLETS];
} TankState;
//-----------------------------------

//-----------------------------------
// Invaders
typedef enum invader_move {
    InvaderMove_Left,
    InvaderMove_Right,
    InvaderMove_Down,
} InvaderMove;

typedef struct invader_state {
    Rect target;
    bool active;
    float32 moveDelay;
    float32 fireDelay;
    float32 deathTime;
    InvaderMove queuedMove;
    int invaderType;
    int frame;
    int bullets[MAX_INVADER_BULLETS];
} InvaderState;
//-----------------------------------

//-----------------------------------
// Game
typedef struct play_state {
    TankState tank;
    ShieldState shields[MAX_SHIELDS];
    BulletState bullets[MAX_BULLETS];
    InvaderState invaders[MAX_INVADERS];
    InvaderMove moveQueue[INVADER_MOVE_QUEUE_SIZE];
    int moveIndex;
    float32 moveDelay;
} PlayState;

typedef struct input_state {
    bool prevKeys[SDL_NUM_SCANCODES];
    bool currKeys[SDL_NUM_SCANCODES];
} InputState;

typedef struct game_state {
    PlayState play;
    InputState input;
} GameState;

typedef struct game {
    SDL_Window* window;
    SDL_Renderer* renderer;
    GameState* gameState;
} Game;
//-----------------------------------

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Static data tables
static SDL_Color cColorPalette[2] = {
    { 255, 255, 255, 255 },
    { 0, 255, 0, 255 },
};

static uint8 cTankImageData[13 * 8] = {
    0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 2, 2, 2, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 2, 2, 2, 0, 0, 0, 0, 0,
    0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
};

static uint8 cInvader1Frame1ImageData[12 * 8] = {
    0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0,
    0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0,
    0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0,
};

static uint8 cInvader1Frame2ImageData[12 * 8] = {
    0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0,
    0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
};

static uint8 cInvader2Frame1ImageData[13 * 8] = {
    0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
    0, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0,
    0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0,
};

static uint8 cInvader2Frame2ImageData[13 * 8] = {
    0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0,
    0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0,
    0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0,
    0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
    0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
};

static uint8 cInvader3Frame1ImageData[8 * 8] = {
    0, 0, 0, 1, 1, 0, 0, 0,
    0, 0, 1, 1, 1, 1, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 0,
    1, 1, 0, 1, 1, 0, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    0, 1, 0, 1, 1, 0, 1, 0,
    1, 0, 0, 0, 0, 0, 0, 1,
    0, 1, 0, 0, 0, 0, 1, 0,
};

static uint8 cInvader3Frame2ImageData[8 * 8] = {
    0, 0, 0, 1, 1, 0, 0, 0,
    0, 0, 1, 1, 1, 1, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 0,
    1, 1, 0, 1, 1, 0, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 1, 0, 0, 1, 0, 0,
    0, 1, 0, 1, 1, 0, 1, 0,
    1, 0, 1, 0, 0, 1, 0, 1,
};

static uint8 cExplosionImageData[13 * 8] = {
    0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0,
    0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0,
    0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
    0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0,
    0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0,
};

static int cInvaderTextureTable[3] = { 1, 3, 5 };
static int cInvaderWidthTable[3] = { 12, 13, 8 };
static int cInvaderHeightTable[3] = { 8, 8, 8 };

static uint8 cUfoImageData[16 * 7] = {
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
    0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0,
    0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
};

static uint8 cTankBulletImageData[1 * 3] = {
    2,
    2,
    2,
};

static uint8 cInvaderBulletFrame1ImageData[3 * 5] = {
    0, 1, 0,
    1, 0, 0,
    0, 1, 0,
    0, 0, 1,
    0, 1, 0,
};

static uint8 cInvaderBulletFrame2ImageData[3 * 5] = {
    0, 1, 0,
    0, 0, 1,
    0, 1, 0,
    1, 0, 0,
    0, 1, 0,
};

static uint8 cShieldImageData[18 * 14] = {
    0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0,
    0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0,
    0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2,
    2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2,
    2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2,
    2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2,
    2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2,
    2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2,
};

static const uint8 cTankTexture = 0;
static const uint8 cInvader1Frame1Texture = 1;
static const uint8 cInvader1Frame2Texture = 2;
static const uint8 cInvader2Frame1Texture = 3;
static const uint8 cInvader2Frame2Texture = 4;
static const uint8 cInvader3Frame1Texture = 5;
static const uint8 cInvader3Frame2Texture = 6;
static const uint8 cUfoTexture = 7;
static const uint8 cTankBulletTexture = 8;
static const uint8 cInvaderBulletFrame1Texture = 9;
static const uint8 cInvaderBulletFrame2Texture = 10;
static const uint8 cExplosionTexture = 11;
static const uint8 cShieldTexture = 12;
////////////////////////////////////////////////////////////////////////////////

void game_init(Game* self);
void game_update(Game* self, float32 dt);
void game_render(Game* self);

void play_reset(PlayState* self);
void tank_reset(TankState* self);
void bullet_reset(BulletState* self);
void bullet_remove(BulletState* self);
void bullet_create(BulletState* self, int x, int y, int bulletType, int* ownerHandle);
void invader_reset(InvaderState* self, int x, int y, int invaderType);
void shield_damage(ShieldState* self, Game* game, int32* indices, int32 count);

void input_reset(InputState* self);
void input_update(InputState* self);
void input_set_key(InputState* self, int scancode, bool isDown);
bool input_get_key(InputState* self, int scancode);
bool input_get_down(InputState* self, int scancode);
bool input_get_up(InputState* self, int scancode);

float32 range_rand(Range* range);
void bounds_grow(Bounds* self, Point* point);
Bounds bounds_from_rect(Rect* rect);
IBounds ibounds_from_rect(Rect* rect);
void ibounds_extract_union(IBounds* a, IBounds* b, IBounds* dest);
Rect rect_from_bounds(Bounds* bounds);
bool rect_intersects(Rect* a, Rect* b);
bool px_to_px_intersect(Rect* a, Rect* b, PaletteTexture* texA, PaletteTexture* texB, PxCollisionData* data);
void rect_to_sdl(Rect* rect, SDL_Rect* dest);
float32 lerp(float32 a, float32 b, float32 t);
float32 clamp(float32 v, float32 min, float32 max);
float32 clamp01(float32 v);
float32 clamp_range(float32 v, Range* range);
float32 lerp_range(Range* range, float32 t);
float32 lerp_clamp_range(Range* range, float32 t);

PaletteTexture create_palette_image_texture(SDL_Renderer* renderer, uint8* data, int width, int height, SDL_Color* palette);

PaletteTexture g_textures[MAX_TEXTURES] = { 0 };

void configure(Config* config) {
    config->tankSpeed = 50.f;
    config->tankBulletSpeed = 350.f;
    config->tankFireOffset.x = 0;
    config->tankFireOffset.y = -4;

    config->invaderMoveDelay.min = 0.01f;
    config->invaderMoveDelay.max = 1.0f;
    config->invaderRowDelay.min = 0.f;
    config->invaderRowDelay.max = 0.2f;
    config->invaderFireDelay.min = 2.0f;
    config->invaderFireDelay.max = 6.0f;
    config->invaderMoveAmount = 4;
    config->invaderBulletSpeed = 150.f;
    config->invaderDeathTime = 0.5f;
}

void rebuild_textures(SDL_Renderer* renderer, PaletteTexture* textures, size_t count, SDL_Color* palette) {
    for (int i = 0; i < count; ++i) {
        if (textures[i].texture != NULL) {
            SDL_DestroyTexture(textures[i].texture);
        }
    }

    int texidx = 0;
    textures[texidx++] = create_palette_image_texture(renderer, cTankImageData, 13, 8, palette);                    // 00
    textures[texidx++] = create_palette_image_texture(renderer, cInvader1Frame1ImageData, 12, 8, palette);          // 01
    textures[texidx++] = create_palette_image_texture(renderer, cInvader1Frame2ImageData, 12, 8, palette);          // 02
    textures[texidx++] = create_palette_image_texture(renderer, cInvader2Frame1ImageData, 13, 8, palette);          // 03
    textures[texidx++] = create_palette_image_texture(renderer, cInvader2Frame2ImageData, 13, 8, palette);          // 04
    textures[texidx++] = create_palette_image_texture(renderer, cInvader3Frame1ImageData, 8, 8, palette);           // 05
    textures[texidx++] = create_palette_image_texture(renderer, cInvader3Frame2ImageData, 8, 8, palette);           // 06
    textures[texidx++] = create_palette_image_texture(renderer, cUfoImageData, 16, 7, palette);                     // 07
    textures[texidx++] = create_palette_image_texture(renderer, cTankBulletImageData, 1, 3, palette);               // 08
    textures[texidx++] = create_palette_image_texture(renderer, cInvaderBulletFrame1ImageData, 3, 5, palette);      // 09
    textures[texidx++] = create_palette_image_texture(renderer, cInvaderBulletFrame2ImageData, 3, 5, palette);      // 10
    textures[texidx++] = create_palette_image_texture(renderer, cExplosionImageData, 13, 8, palette);               // 11
    textures[texidx++] = create_palette_image_texture(renderer, cShieldImageData, 18, 14, palette);                 // 12
}

int main(int argc, char* argv[]) {
    configure(&g_config);

    srand(time(NULL));

    SDL_Window* window = SDL_CreateWindow("Vasion", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1080, 720, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);

    SDL_Texture* screenTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, cScreenWidth, cScreenHeight);

    SDL_RenderSetLogicalSize(renderer, cScreenWidth, cScreenHeight);

    rebuild_textures(renderer, g_textures, MAX_TEXTURES, cColorPalette);

    GameState gameState;

    Game game;
    game.window = window;
    game.renderer = renderer;
    game.gameState = &gameState;

    game_init(&game);

    uint64 time_prev_ticks = 0;
    float32 time_dt = 0.f;

    bool isRunning = true;
    while (isRunning) {
        input_update(&gameState.input);

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    isRunning = false;
                    break;

                case SDL_KEYDOWN:
                    input_set_key(&gameState.input, event.key.keysym.scancode, true);

                    if (event.key.keysym.scancode == SDL_SCANCODE_D) {
                        int index = rand() % MAX_INVADERS;
                        while (!gameState.play.invaders[index].active) {
                            index = rand() % MAX_INVADERS;
                        }
                        gameState.play.invaders[index].active = false;
                    }
                    break;

                case SDL_KEYUP:
                    input_set_key(&gameState.input, event.key.keysym.scancode, false);
                    break;
            }
        }

        if (input_get_down(&gameState.input, SDL_SCANCODE_ESCAPE)) {
            isRunning = false;
        }

        {
            uint64 ticks = SDL_GetPerformanceCounter();
            uint64 frequency = SDL_GetPerformanceFrequency();

            if (time_prev_ticks == 0) {
                time_prev_ticks = ticks;
            }

            uint64 diff = (ticks - time_prev_ticks) * 1000000000 / frequency;
            time_prev_ticks = ticks;

            time_dt = (float32)diff / 1000000000.f;
        }

        game_update(&game, time_dt);

        // render to the render texture
        {
            SDL_SetRenderTarget(renderer, screenTexture);
            SDL_SetRenderDrawColor(renderer, 32, 32, 48, 255);
            SDL_RenderClear(renderer);

            game_render(&game);
        }

        // render the render texture to the window
        {
            SDL_SetRenderTarget(renderer, NULL);

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            //SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, screenTexture, NULL, NULL);

            SDL_RenderPresent(renderer);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}

void game_init(Game* self) {
    GameState* state = self->gameState;
    for (int i = 0; i < MAX_SHIELDS; ++i) {
        state->play.shields[i].texture.texture = NULL;
        state->play.shields[i].texture.data = NULL;
    }

    play_reset(&state->play);
    input_reset(&state->input);
}

void game_update(Game* self, float32 dt) {
    GameState* state = self->gameState;
    InputState* input = &state->input;

    // Tank Movement
    TankState* tank = &state->play.tank;
    {
        float32 speed = g_config.tankSpeed * dt;
        if (input_get_key(input, KEY_LEFT)) {
            tank->target.position.x -= speed;
        }
        if (input_get_key(input, KEY_RIGHT)) {
            tank->target.position.x += speed;
        }

        float leftBound = tank->target.width / 2;
        if (tank->target.position.x < leftBound) {
            tank->target.position.x = leftBound;
        }
        float rightBound = cScreenWidth - tank->target.width / 2;
        if (tank->target.position.x > rightBound) {
            tank->target.position.x = rightBound;
        }
    }

    // Tank Shooting
    {
        bool requestShot = input_get_down(input, KEY_FIRE);
        if (requestShot) {
            BulletState* bullet = NULL;
            int handle = 0;
            for (int i = 0; i < MAX_TANK_BULLETS; ++i) {
                if (tank->bullets[i] < 0) {
                    for (int j = 0; j < MAX_BULLETS; ++j) {
                        if (!state->play.bullets[j].active) {
                            tank->bullets[i] = j;
                            bullet = &state->play.bullets[j];
                        }
                    }
                    handle = i;
                    break;
                }
            }

            if (bullet) {
                bullet_create(bullet,
                    tank->target.position.x + g_config.tankFireOffset.x,
                    tank->target.position.y + g_config.tankFireOffset.y,
                    0,
                    &tank->bullets[handle]);
            }
        }
    }

    // Invaders
    {
        state->play.moveDelay -= dt;

        // figure out boundaries of entire swarm
        int aliveInvaders = 0;
        Bounds invaderBounds = { 999999, -999999, 999999, -999999 };
        for (int i = 0; i < MAX_INVADERS; ++i) {
            InvaderState* invader = &state->play.invaders[i];
            if (invader->active) {
                ++aliveInvaders;
                bounds_grow(&invaderBounds, &invader->target.position);
            }
        }

        // time for the next move
        if (state->play.moveDelay <= 0.f) {
            // using previous two moves figure out which move is appropriate
            InvaderMove prevMove1 = state->play.moveQueue[(state->play.moveIndex - 0) % INVADER_MOVE_QUEUE_SIZE];
            InvaderMove prevMove2 = state->play.moveQueue[(state->play.moveIndex - 1) % INVADER_MOVE_QUEUE_SIZE];

            InvaderMove move = prevMove1;

            switch (prevMove1) {
                case InvaderMove_Right:
                    if (invaderBounds.right >= INVADER_BOUNDARY_RIGHT) {
                        move = InvaderMove_Down;
                    }
                    break;

                case InvaderMove_Left:
                    if (invaderBounds.left <= INVADER_BOUNDARY_LEFT) {
                        move = InvaderMove_Down;
                    }
                    break;

                case InvaderMove_Down:
                    if (prevMove2 == InvaderMove_Left) {
                        move = InvaderMove_Right;
                    }
                    else {
                        move = InvaderMove_Left;
                    }
                    break;
            }

            float32 alivePerc = (float32)aliveInvaders / MAX_INVADERS;

            // update move queue and move delay
            state->play.moveIndex++;
            int index = state->play.moveIndex % INVADER_MOVE_QUEUE_SIZE;
            state->play.moveQueue[index] = move;
            state->play.moveDelay += lerp(g_config.invaderMoveDelay.min, g_config.invaderMoveDelay.max, alivePerc);

            // tell all invaders what their next move is and how long to wait until doing it
            for (int i = 0; i < MAX_INVADERS; ++i) {
                InvaderState* invader = &state->play.invaders[i];
                if (invader->active) {
                    invader->frame++;
                    switch (move) {
                        case InvaderMove_Down: invader->target.position.y += g_config.invaderMoveAmount; break;
                        case InvaderMove_Left: invader->target.position.x -= g_config.invaderMoveAmount; break;
                        case InvaderMove_Right: invader->target.position.x += g_config.invaderMoveAmount; break;
                    }
                }
            }
        }
    }

    // Invader bullet firing
    {
        for (int i = 0; i < MAX_INVADERS; ++i) {
            InvaderState* invader = &state->play.invaders[i];
            if (invader->active) {
                invader->fireDelay -= dt;
                if (invader->fireDelay <= 0.f) {
                    BulletState* bullet = NULL;
                    int handle = 0;
                    for (int j = 0; j < MAX_INVADER_BULLETS; ++j) {
                        if (invader->bullets[i] < 0) {
                            for (int k = 0; k < MAX_BULLETS; ++k) {
                                if (!state->play.bullets[k].active) {
                                    invader->bullets[i] = k;
                                    bullet = &state->play.bullets[k];
                                }
                            }
                            handle = j;
                            break;
                        }
                    }

                    if (bullet) {
                        bullet_create(bullet,
                            invader->target.position.x + 0,
                            invader->target.position.y + 0,
                            1,
                            &invader->bullets[handle]);
                        invader->fireDelay = range_rand(&g_config.invaderFireDelay);
                    }
                }
            }
            else {
                if (invader->deathTime > 0) {
                    invader->deathTime -= dt;
                }
            }
        }
    }

    // Bullet updates
    {
        for (int i = 0; i < MAX_BULLETS; ++i) {
            BulletState* bullet = &state->play.bullets[i];
            if (bullet->active) {
                bullet->frame++;
                float32 speed = (bullet->direction > 0) ? g_config.invaderBulletSpeed : g_config.tankBulletSpeed;
                bullet->target.position.y += speed * bullet->direction * dt;
                if (bullet->target.position.y < 0 || bullet->target.position.y > cScreenHeight + 4) {
                    bullet_remove(bullet);
                }

                if (bullet->direction < 0) {
                    for (int j = 0; j < MAX_INVADERS; ++j) {
                        InvaderState* invader = &state->play.invaders[j];
                        if (invader->active) {
                            if (rect_intersects(&bullet->target, &invader->target)) {
                                bullet_remove(bullet);
                                invader->active = false;
                                invader->deathTime = g_config.invaderDeathTime;
                            }
                        }
                    }
                }
                else if (bullet->direction > 0) {
                    if (rect_intersects(&bullet->target, &tank->target)) {
                        bullet_remove(bullet);
                    }
                }
            }
        }
    }

    // Shield stuff
    {
        for (int i = 0; i < MAX_SHIELDS; ++i) {
            ShieldState* shield = &state->play.shields[i];
            for (int j = 0; j < MAX_BULLETS; ++j) {
                BulletState* bullet = &state->play.bullets[j];
                if (!bullet->active) continue;

                int texIdx = bullet->baseTexture + ((bullet->frame / 30) % bullet->frameCount);
                PxCollisionData collData;
                if (px_to_px_intersect(&shield->target,
                    &bullet->target,
                    &g_textures[cShieldTexture],
                    &g_textures[texIdx],
                    &collData)) {
                    shield_damage(shield, self, &collData.pixelA, 1);
                    bullet_remove(bullet);
                }
            }
        }
    }
}

void game_render(Game* self) {
    GameState* state = self->gameState;
    SDL_Renderer* renderer = self->renderer;

    TankState* tank = &state->play.tank;
    {
        SDL_Rect r;
        rect_to_sdl(&tank->target, &r);
        SDL_RenderCopy(renderer, g_textures[0].texture, NULL, &r);
    }

    for (int i = 0; i < MAX_SHIELDS; ++i) {
        ShieldState* shield = &state->play.shields[i];
        SDL_Rect r;
        rect_to_sdl(&shield->target, &r);
        SDL_RenderCopy(renderer, g_textures[12].texture, NULL, &r);
    }

    for (int i = 0; i < MAX_INVADERS; ++i) {
        InvaderState* invader = &state->play.invaders[i];
        if (invader->active) {
            int baseIndex = cInvaderTextureTable[invader->invaderType];
            int textureIndex = baseIndex + (invader->frame & 0x1);
            SDL_Rect r;
            rect_to_sdl(&invader->target, &r);
            SDL_RenderCopy(renderer, g_textures[textureIndex].texture, NULL, &r);
        }
        else {
            if (invader->deathTime > 0.f) {
                SDL_Rect r;
                rect_to_sdl(&invader->target, &r);
                SDL_RenderCopy(renderer, g_textures[11].texture, NULL, &r);
            }
        }
    }

    for (int i = 0; i < MAX_BULLETS; ++i) {
        BulletState* bullet = &state->play.bullets[i];
        if (bullet->active) {
            int texIdx = bullet->baseTexture + ((bullet->frame / 30) % bullet->frameCount);
            SDL_Rect r;
            rect_to_sdl(&bullet->target, &r);
            SDL_RenderCopy(renderer, g_textures[texIdx].texture, NULL, &r);
        }
    }
}

void play_reset(PlayState* self) {
    tank_reset(&self->tank);

    for (int i = 0; i < MAX_SHIELDS; ++i) {
        ShieldState* shield = &self->shields[i];
        Rect* target = &shield->target;
        target->position.x = 43 + i * (18 + 33 + ((i - 1) % 2));
        target->position.y = cScreenHeight - 40;
        target->width = 18;
        target->height = 14;
        if (shield->texture.texture) {
            SDL_DestroyTexture(shield->texture.texture);
            shield->texture.texture = NULL;
        }
        if (shield->texture.data) {
            free(shield->texture.data);
            shield->texture.data = NULL;
        }
    }

    for (int i = 0; i < MAX_BULLETS; ++i) {
        bullet_reset(&self->bullets[i]);
    }

    const int spacing = 4;
    const int offsetX = 10;
    const int offsetY = 20;
    for (int i = 0; i < MAX_INVADERS; ++i) {
        int row = i / INVADER_COLS;
        int col = i % INVADER_COLS;
        int x = col * (spacing + 13) + offsetX;
        int y = row * (spacing + 8) + offsetY;
        int invaderType = 0;
        switch (row) {
            case 0: invaderType = 2; break;
            case 1:
            case 2: invaderType = 1; break;
            default: break;
        }
        invader_reset(&self->invaders[i], x, y, invaderType);
    }
    self->moveDelay = g_config.invaderMoveDelay.max;
    self->moveIndex = 0;
    for (int i = 0; i < INVADER_MOVE_QUEUE_SIZE; ++i) {
        self->moveQueue[i] = InvaderMove_Right;
    }
}

void tank_reset(TankState* self) {
    self->target.position.x = cScreenWidth / 2;
    self->target.position.y = cScreenHeight - 8;
    self->target.width = 13;
    self->target.height = 8;
    self->mode = TankMode_Active;
    for (int i = 0; i < MAX_TANK_BULLETS; ++i) {
        self->bullets[i] = -1;
    }
}

void bullet_reset(BulletState* self) {
    self->target.position.x = 0;
    self->target.position.y = 0;
    self->target.width = 0;
    self->target.height = 0;
    self->active = false;
    self->frame = 0;
    self->frameCount = 1;
    self->baseTexture = 8;
    self->direction = -1;
    self->ownerHandle = NULL;
}

void bullet_remove(BulletState* self) {
    self->active = false;
    if (self->ownerHandle) {
        *self->ownerHandle = -1;
        self->ownerHandle = NULL;
    }
}

void bullet_create(BulletState* self, int x, int y, int bulletType, int* ownerHandle) {
    self->target.position.x = x;
    self->target.position.y = y;
    self->active = true;
    self->frame = 0;
    self->ownerHandle = ownerHandle;

    switch (bulletType) {
        default:
        case 0:
            self->target.width = 1;
            self->target.height = 3;
            self->frameCount = 1;
            self->baseTexture = 8;
            self->direction = -1;
            break;

        case 1:
            self->target.width = 3;
            self->target.height = 5;
            self->frameCount = 2;
            self->baseTexture = 9;
            self->direction = 1;
            break;
    }
}

void invader_reset(InvaderState* self, int x, int y, int invaderType) {
    self->target.position.x = x;
    self->target.position.y = y;
    self->target.width = cInvaderWidthTable[invaderType];
    self->target.height = cInvaderHeightTable[invaderType];
    self->active = true;
    self->moveDelay = 0.f;
    self->fireDelay = range_rand(&g_config.invaderFireDelay);
    self->invaderType = invaderType;
    self->frame = 0;
    for (int i = 0; i < MAX_INVADER_BULLETS; ++i) {
        self->bullets[i] = -1;
    }
}

void shield_damage(ShieldState* self, Game* game, int32* indices, int32 count) {
    if (!self->texture.texture) {
        self->texture = create_palette_image_texture(game->renderer,
            cShieldImageData,
            18, 14,
            cColorPalette);
    }
}

void input_reset(InputState* self) {
    for (int i = 0; i < 512; ++i) {
        self->prevKeys[i] = false;
        self->currKeys[i] = false;
    }
}

void input_update(InputState* self) {
    for (int i = 0; i < 512; ++i) {
        self->prevKeys[i] = self->currKeys[i];
    }
}

void input_set_key(InputState* self, int scancode, bool isDown) {
    self->currKeys[scancode] = isDown;
}

bool input_get_key(InputState* self, int scancode) {
    return self->currKeys[scancode];
}

bool input_get_down(InputState* self, int scancode) {
    return self->currKeys[scancode] && !self->prevKeys[scancode];
}

bool input_get_up(InputState* self, int scancode) {
    return !self->currKeys[scancode] && self->prevKeys[scancode];
}

PaletteTexture create_palette_image_texture(SDL_Renderer* renderer, uint8* data, int width, int height, SDL_Color* palette) {
    SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);

    const int size = width * height;
    for (int i = 0; i < size; ++i) {
        SDL_Color color = {
            0, 0, 0, 0,
        };

        uint8 value = data[i];
        if (value > 0) {
            color = palette[value - 1];
        }

        uint32 packedColor = SDL_MapRGBA(surface->format, color.r, color.g, color.b, color.a);

        SDL_Rect r = {
            i % width, i / width,
            1, 1
        };
        SDL_FillRect(surface, &r, packedColor);
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    PaletteTexture result = {
        texture, data, width, height,
    };

    return result;
}

float32 range_rand(Range* range) {
    return lerp(range->min, range->max, (float32)rand() / (float32)RAND_MAX);
}

void bounds_grow(Bounds* self, Point* point) {
    if (point->x < self->left) {
        self->left = point->x;
    }
    if (point->x > self->right) {
        self->right = point->x;
    }
    if (point->y < self->top) {
        self->top = point->y;
    }
    if (point->y > self->bottom) {
        self->bottom = point->y;
    }
}

Bounds bounds_from_rect(Rect* rect) {
    Bounds result = {
        rect->position.x - rect->width / 2,
        rect->position.x + rect->width / 2,
        rect->position.y - rect->height / 2,
        rect->position.y + rect->height / 2,
    };
    return result;
}

IBounds ibounds_from_rect(Rect* rect) {
    IBounds result = {
        (int32)(rect->position.x - rect->width / 2),
        (int32)(rect->position.x + rect->width / 2),
        (int32)(rect->position.y - rect->height / 2),
        (int32)(rect->position.y + rect->height / 2),
    };
    return result;
}

void ibounds_extract_union(IBounds* a, IBounds* b, IBounds* dest) {
    dest->left = max(a->left, b->left);
    dest->right = min(a->right, b->right);
    dest->top = max(a->top, b->top);
    dest->bottom = min(a->bottom, b->bottom);
    if (dest->left > dest->right) {
        dest->left = 0;
        dest->right = 0;
    }
    if (dest->top > dest->bottom) {
        dest->top = 0;
        dest->bottom = 0;
    }
}

Rect rect_from_bounds(Bounds* bounds) {
    Rect result = {
        {
            (bounds->left + bounds->right) / 2.f,
            (bounds->top + bounds->bottom) / 2.f,
        },
        fabs(bounds->right - bounds->left),
        fabs(bounds->bottom - bounds->top),
    };
    return result;
}

bool rect_intersects(Rect* a, Rect* b) {
    Bounds ab, bb;
    ab = bounds_from_rect(a);
    bb = bounds_from_rect(b);
    return ab.top <= bb.bottom && ab.bottom >= bb.top &&
        ab.left <= bb.right && ab.right >= bb.left;
}

bool px_to_px_intersect(Rect* a, Rect* b, PaletteTexture* texA, PaletteTexture* texB, PxCollisionData* data) {
    if (data) {
        data->pixelA = 0;
        data->pixelB = 0;
    }

    // early out if we're not intersecting rectangles at all
    if (!rect_intersects(a, b)) {
        return false;
    }

    IBounds ai = ibounds_from_rect(a);
    IBounds bi = ibounds_from_rect(b);

    IBounds areaA, areaB;

    ibounds_extract_union(&ai, &bi, &areaA);
    ibounds_extract_union(&bi, &ai, &areaB);

    IPoint topLeftA = { areaA.left, areaA.top };
    IPoint topLeftB = { areaB.left, areaB.top };

    for (int32 row = ai.top; row <= ai.bottom; ++row) {
        for (int32 col = ai.left; col <= ai.right; ++col) {
            int32 ar = row - topLeftA.y;
            int32 ac = col - topLeftA.x;
            int32 br = row - topLeftB.y;
            int32 bc = col - topLeftB.x;

            int32 indexA = ar * texA->width + ac;
            int32 indexB = br * texB->width + bc;

            if (texA->data[indexA] && texB->data[indexB]) {
                if (data) {
                    data->pixelA = indexA;
                    data->pixelB = indexB;
                    return true;
                }
            }
        }
    }

    return false;
}

void rect_to_sdl(Rect* rect, SDL_Rect* dest) {
    int hw = rect->width / 2;
    int hh = rect->height / 2;
    dest->x = (int)rect->position.x - hw;
    dest->y = (int)rect->position.y - hh;
    dest->w = (int)rect->width;
    dest->h = (int)rect->height;
}

float32 lerp(float32 a, float32 b, float32 t) {
    return (b - a) * t + a;
}

float32 clamp(float32 v, float32 min, float32 max) {
    return (v < min) ? min : (v > max) ? max : v;
}

float32 clamp01(float32 v) {
    return clamp(v, 0.f, 1.f);
}

float32 clamp_range(float32 v, Range* range) {
    return clamp(v, range->min, range->max);
}

float32 lerp_range(Range* range, float32 t) {
    return lerp(range->min, range->max, t);
}

float32 lerp_clamp_range(Range* range, float32 t) {
    return lerp_range(range, clamp01(t));
}