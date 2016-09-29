#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL.h>

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

static SDL_Color cTankPalette[1] = {
    { 0, 255, 0, 255 },
};

static SDL_Color cInvaderPalette[1] = {
    { 255, 255, 255, 255 },
};

static uint8 cTankImageData[13 * 8] = {
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

static uint8 cInvader1ImageData[13 * 8] = {
    0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
    0, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0,
    0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0,
};

static const int cScreenWidth = 240;
static const int cScreenHeight = 160;

#define SHIELD_CHUNK_COUNT 25
#define SHIELD_COUNT 4
#define INVADER_ROWS 5
#define INVADER_COLS 11
#define INVADER_WIDTH 13
#define INVADER_HEIGHT 8
#define INVADER_BOUNDARY_LEFT 10
#define INVADER_BOUNDARY_RIGHT (cScreenWidth - INVADER_BOUNDARY_LEFT)
#define MAX_INVADERS (INVADER_ROWS * INVADER_COLS)
#define MAX_TANK_BULLETS 2

#define KEY_LEFT SDL_SCANCODE_LEFT
#define KEY_RIGHT SDL_SCANCODE_RIGHT
#define KEY_FIRE SDL_SCANCODE_Z

typedef struct config {
    float32 tankSpeed;
    float32 invaderMinMoveDelay;
    float32 invaderMaxMoveDelay;
    float32 invaderMoveAmount;
} Config;

Config g_config;

typedef enum bullet_source {
    BulletSource_Tank,
    BulletSource_Invader,
} BulletSource;

typedef struct bullet_state {
    float32 x, y;
    float32 w, h;
    bool active;
    BulletSource source;
} BulletState;

typedef struct shield_state {
    float32 x, y;
    bool chunks[SHIELD_CHUNK_COUNT];
} ShieldState;

typedef enum tank_mode {
    TankMode_Active,
    TankMode_Dead,
} TankMode;

typedef struct tank_state {
    float32 x, y;
    float32 w, h;
    TankMode mode;
    BulletState bullets[MAX_TANK_BULLETS];
} TankState;


#define INVADER_MOVE_QUEUE_SIZE 3

typedef enum invader_move {
    InvaderMove_Left,
    InvaderMove_Right,
    InvaderMove_Down,
} InvaderMove;

typedef struct invader_state {
    float32 x, y;
    float32 w, h;
    bool active;
    float moveDelay;
    InvaderMove moveQueue[INVADER_MOVE_QUEUE_SIZE];
    int moveHead, moveTail;
    BulletState bullet;
} InvaderState;

typedef struct play_state {
    TankState tank;
    ShieldState shields[SHIELD_COUNT];
    InvaderState invaders[MAX_INVADERS];
    int invaderDirection;
} PlayState;

typedef struct input_state {
    bool prevKeys[SDL_NUM_SCANCODES];
    bool currKeys[SDL_NUM_SCANCODES];
} InputState;

typedef struct game_state {
    PlayState play;
    InputState input;
} GameState;

typedef struct bounds {
    float32 left, right, top, bottom;
} Bounds;

void game_init(GameState* self);
void game_update(GameState* self, float32 dt);
void game_render(GameState* self, SDL_Renderer* renderer);

void play_reset(PlayState* self);
void tank_reset(TankState* self);
void bullet_reset(BulletState* self, BulletSource bulletSource);
void invader_reset(InvaderState* self, int x, int y);

void input_reset(InputState* self);
void input_update(InputState* self);
void input_set_key(InputState* self, int scancode, bool isDown);
bool input_get_key(InputState* self, int scancode);
bool input_get_down(InputState* self, int scancode);
bool input_get_up(InputState* self, int scancode);

void bounds_grow(Bounds* self, float32 x, float32 y);
float32 lerp(float32 a, float32 b, float32 t);

SDL_Texture* create_palette_image_texture(SDL_Renderer* renderer, uint8* data, int width, int height, SDL_Color* palette);

SDL_Texture* g_tankTexture;
SDL_Texture* g_invader1Texture;

int main(int argc, char* argv[]) {
    g_config.tankSpeed = 50.f;
    g_config.invaderMinMoveDelay = 0.1f;
    g_config.invaderMaxMoveDelay = 1.f;
    g_config.invaderMoveAmount = 1;

    srand(time(NULL));

    SDL_Window* window = SDL_CreateWindow("Vasion", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 480, 320, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);

    SDL_Texture* screenTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, cScreenWidth, cScreenHeight);

    SDL_RenderSetLogicalSize(renderer, cScreenWidth, cScreenHeight);

    g_tankTexture = create_palette_image_texture(renderer, cTankImageData, 13, 8, cTankPalette);
    g_invader1Texture = create_palette_image_texture(renderer, cInvader1ImageData, 13, 8, cInvaderPalette);

    GameState gameState;

    game_init(&gameState);

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

        game_update(&gameState, time_dt);

        // render to the render texture
        {
            SDL_SetRenderTarget(renderer, screenTexture);
            SDL_SetRenderDrawColor(renderer, 32, 32, 48, 255);
            SDL_RenderClear(renderer);

            game_render(&gameState, renderer);
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

void game_init(GameState* self) {
    play_reset(&self->play);
    input_reset(&self->input);
}

void game_update(GameState* self, float32 dt) {
    InputState* input = &self->input;

    TankState* tank = &self->play.tank;
    {
        float32 speed = g_config.tankSpeed * dt;
        if (input_get_key(input, KEY_LEFT)) {
            tank->x -= speed;
        }
        if (input_get_key(input, KEY_RIGHT)) {
            tank->x += speed;
        }

        float leftBound = tank->w / 2;
        if (tank->x < leftBound) {
            tank->x = leftBound;
        }
        float rightBound = cScreenWidth - tank->w / 2;
        if (tank->x > rightBound) {
            tank->x = rightBound;
        }
    }


    {
        if (input_get_down(input, SDL_SCANCODE_D)) {
            self->play.invaders[rand() % MAX_INVADERS].active = false;
        }

        Bounds invaderBounds = { 999999, -999999, 999999, -999999 };
        int aliveInvaders = 0;
        for (int i = 0; i < MAX_INVADERS; ++i) {
            InvaderState* invader = &self->play.invaders[i];
            if (invader->active) {
                ++aliveInvaders;
                bounds_grow(&invaderBounds, invader->x, invader->y);
            }
        }

        bool doDown = false;
        if (self->play.invaderDirection > 0) {
            if (invaderBounds.right >= INVADER_BOUNDARY_RIGHT) {
                doDown = true;
                self->play.invaderDirection = -1;
            }
        }
        else if (self->play.invaderDirection < 0) {
            if (invaderBounds.left <= INVADER_BOUNDARY_LEFT) {
                doDown = true;
                self->play.invaderDirection = 1;
            }
        }

        float32 speed = self->play.invaderDirection * g_config.invaderMoveAmount;

        for (int i = 0; i < MAX_INVADERS; ++i) {
            InvaderState* invader = &self->play.invaders[i];
            if (invader->active) {
                invader->moveDelay -= dt;
                if (invader->moveDelay <= 0) {
                    invader->x += speed;
                    invader->moveDelay = lerp(g_config.invaderMaxMoveDelay, g_config.invaderMinMoveDelay, aliveInvaders / MAX_INVADERS);
                }
            }
        }
    }
}

void game_render(GameState* self, SDL_Renderer* renderer) {
    TankState* tank = &self->play.tank;
    {
        int hw = tank->w / 2;
        int hh = tank->h / 2;
        SDL_Rect r = {
            tank->x - hw, tank->y - hh,
            tank->w, tank->h,
        };
        SDL_RenderCopy(renderer, g_tankTexture, NULL, &r);
    }

    for (int i = 0; i < MAX_INVADERS; ++i) {
        InvaderState* invader = &self->play.invaders[i];
        if (invader->active) {
            int hw = invader->w / 2;
            int hh = invader->h / 2;
            SDL_Rect r = {
                invader->x - hw, invader->y - hh,
                invader->w, invader->h,
            };
            SDL_RenderCopy(renderer, g_invader1Texture, NULL, &r);
        }
    }
}

void play_reset(PlayState* self) {
    tank_reset(&self->tank);

    const int spacing = 4;
    const int offsetX = 10;
    const int offsetY = 10;
    for (int i = 0; i < MAX_INVADERS; ++i) {
        int row = i / INVADER_COLS;
        int col = i % INVADER_COLS;
        int x = col * (spacing + 13) + offsetX;
        int y = row * (spacing + 8) + offsetY;
        invader_reset(&self->invaders[i], x, y);
    }

    self->invaderDirection = 1;
}

void tank_reset(TankState* self) {
    self->x = cScreenWidth / 2;
    self->y = cScreenHeight - 8;
    self->w = 13;
    self->h = 8;
    self->mode = TankMode_Active;
    for (int i = 0; i < MAX_TANK_BULLETS; ++i) {
        bullet_reset(&self->bullets[i], BulletSource_Tank);
    }
}

void bullet_reset(BulletState* self, BulletSource bulletSource) {
    self->x = 0;
    self->y = 0;
    self->w = 1;
    self->h = 3;
    self->active = false;
    self->source = bulletSource;
}

void invader_reset(InvaderState* self, int x, int y) {
    self->x = x;
    self->y = y;
    self->w = 13;
    self->h = 8;
    self->active = true;
    self->moveDelay = 0.f;
    bullet_reset(&self->bullet, BulletSource_Invader);
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

SDL_Texture* create_palette_image_texture(SDL_Renderer* renderer, uint8* data, int width, int height, SDL_Color* palette) {
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

    return texture;
}

void bounds_grow(Bounds* self, float32 x, float32 y) {
    if (x < self->left) {
        self->left = x;
    }
    if (x > self->right) {
        self->right = x;
    }
    if (y < self->top) {
        self->top = y;
    }
    if (y > self->bottom) {
        self->bottom = y;
    }
}

float32 lerp(float32 a, float32 b, float32 t) {
    return (b - a) * t + a;
}