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

typedef uint8 byte;

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

static uint8 cTankBulletImageData[3 * 6] = {
    2, 2, 2,
    2, 2, 2,
    2, 2, 2,
    2, 2, 2,
    2, 2, 2,
    2, 2, 2,
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

typedef enum invader_move {
    InvaderMove_Left,
    InvaderMove_Right,
    InvaderMove_Down,
} InvaderMove;

typedef struct invader_state {
    float32 x, y;
    float32 w, h;
    bool active;
    float32 moveDelay;
    InvaderMove queuedMove;
    int invaderType;
    int frame;
    BulletState bullet;
} InvaderState;

#define INVADER_MOVE_QUEUE_SIZE 3

typedef struct play_state {
    TankState tank;
    ShieldState shields[SHIELD_COUNT];
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

typedef struct bounds {
    float32 left, right, top, bottom;
} Bounds;

void game_init(GameState* self);
void game_update(GameState* self, float32 dt);
void game_render(GameState* self, SDL_Renderer* renderer);

void play_reset(PlayState* self);
void tank_reset(TankState* self);
void bullet_reset(BulletState* self, BulletSource bulletSource);
void invader_reset(InvaderState* self, int x, int y, int invaderType);

void input_reset(InputState* self);
void input_update(InputState* self);
void input_set_key(InputState* self, int scancode, bool isDown);
bool input_get_key(InputState* self, int scancode);
bool input_get_down(InputState* self, int scancode);
bool input_get_up(InputState* self, int scancode);

void bounds_grow(Bounds* self, float32 x, float32 y);
float32 lerp(float32 a, float32 b, float32 t);

SDL_Texture* create_palette_image_texture(SDL_Renderer* renderer, uint8* data, int width, int height, SDL_Color* palette);

SDL_Texture* g_textures[20];

int main(int argc, char* argv[]) {
    g_config.tankSpeed = 50.f;
    g_config.invaderMinMoveDelay = 0.01f;
    g_config.invaderMaxMoveDelay = 1.5f;
    g_config.invaderMoveAmount = 4;

    srand(time(NULL));

    SDL_Window* window = SDL_CreateWindow("Vasion", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 480, 320, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);

    SDL_Texture* screenTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, cScreenWidth, cScreenHeight);

    SDL_RenderSetLogicalSize(renderer, cScreenWidth, cScreenHeight);

    {
        int texidx = 0;
        g_textures[texidx++] = create_palette_image_texture(renderer, cTankImageData, 13, 8, cColorPalette);
        g_textures[texidx++] = create_palette_image_texture(renderer, cInvader1Frame1ImageData, 12, 8, cColorPalette);
        g_textures[texidx++] = create_palette_image_texture(renderer, cInvader1Frame2ImageData, 12, 8, cColorPalette);
        g_textures[texidx++] = create_palette_image_texture(renderer, cInvader2Frame1ImageData, 13, 8, cColorPalette);
        g_textures[texidx++] = create_palette_image_texture(renderer, cInvader2Frame2ImageData, 13, 8, cColorPalette);
        g_textures[texidx++] = create_palette_image_texture(renderer, cInvader3Frame1ImageData, 8, 8, cColorPalette);
        g_textures[texidx++] = create_palette_image_texture(renderer, cInvader3Frame2ImageData, 8, 8, cColorPalette);
        g_textures[texidx++] = create_palette_image_texture(renderer, cUfoImageData, 16, 7, cColorPalette);
        g_textures[texidx++] = create_palette_image_texture(renderer, cTankBulletImageData, 3, 6, cColorPalette);
    }

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

const char* get_move_string(InvaderMove move) {
    switch (move) {
        default:
        case InvaderMove_Left: return "L";
        case InvaderMove_Right: return "R";
        case InvaderMove_Down: return "D";
    }
}

void game_update(GameState* self, float32 dt) {
    InputState* input = &self->input;

    // Tank Movement
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

    // Tank Shooting
    {
        bool requestShot = input_get_down(input, KEY_FIRE);
        if (requestShot) {
            BulletState* bullet = NULL;
            for (int i = 0; i < MAX_TANK_BULLETS; ++i) {
                if (!tank->bullets[i].active) {
                    bullet = &tank->bullets[i];
                    break;
                }
            }

            if (bullet) {
                bullet->active = true;
                bullet->x = tank->x;
                bullet->y = tank->y;
            }
        }
    }

    // Invaders
    {
        self->play.moveDelay -= dt;

        // figure out boundaries of entire swarm
        int aliveInvaders = 0;
        Bounds invaderBounds = { 999999, -999999, 999999, -999999 };
        for (int i = 0; i < MAX_INVADERS; ++i) {
            InvaderState* invader = &self->play.invaders[i];
            if (invader->active) {
                ++aliveInvaders;
                bounds_grow(&invaderBounds, invader->x, invader->y);
            }
        }

        // time for the next move
        if (self->play.moveDelay <= 0.f) {
            // using previous two moves figure out which move is appropriate
            InvaderMove prevMove1 = self->play.moveQueue[(self->play.moveIndex - 0) % INVADER_MOVE_QUEUE_SIZE];
            InvaderMove prevMove2 = self->play.moveQueue[(self->play.moveIndex - 1) % INVADER_MOVE_QUEUE_SIZE];

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
            self->play.moveIndex++;
            int index = self->play.moveIndex % INVADER_MOVE_QUEUE_SIZE;
            self->play.moveQueue[index] = move;
            self->play.moveDelay += lerp(g_config.invaderMinMoveDelay, g_config.invaderMaxMoveDelay, alivePerc);

            // tell all invaders what their next move is and how long to wait until doing it
            for (int i = 0; i < MAX_INVADERS; ++i) {
                InvaderState* invader = &self->play.invaders[i];
                if (invader->active) {
                    invader->queuedMove = move;
                    int row = INVADER_ROWS - (i / INVADER_COLS);
                    invader->moveDelay = 0.01f + row * lerp(0.f, 0.2f, alivePerc);
                    // clamp the delay to delay of the next swarm move
                    if (invader->moveDelay > self->play.moveDelay) {
                        invader->moveDelay = self->play.moveDelay;
                    }
                }
            }
        }

        // once an invader's delay is done do the stored move
        for (int i = 0; i < MAX_INVADERS; ++i) {
            InvaderState* invader = &self->play.invaders[i];
            if (invader->active) {
                if (invader->moveDelay > 0.f) {
                    invader->moveDelay -= dt;
                    if (invader->moveDelay <= 0.f) {
                        invader->frame++;
                        switch (invader->queuedMove) {
                            case InvaderMove_Down: invader->y += g_config.invaderMoveAmount; break;
                            case InvaderMove_Left: invader->x -= g_config.invaderMoveAmount; break;
                            case InvaderMove_Right: invader->x += g_config.invaderMoveAmount; break;
                        }
                    }
                }
            }
        }
    }

    // Move bullets
    {
        for (int i = 0; i < MAX_TANK_BULLETS; ++i) {
            BulletState* bullet = &tank->bullets[i];
            if (bullet->active) {
                bullet->y -= 200.f * dt;
                if (bullet->y < 0) {
                    bullet->active = false;
                }

                float32 bltLeft = bullet->x - bullet->w / 2;
                float32 bltRight = bullet->x + bullet->w / 2;
                float32 bltTop = bullet->y - bullet->h / 2;
                float32 bltBottom = bullet->y + bullet->h / 2;
                for (int j = 0; j < MAX_INVADERS; ++j) {
                    InvaderState* invader = &self->play.invaders[j];
                    if (invader->active) {
                        float32 invLeft = invader->x - invader->w / 2;
                        float32 invRight = invader->x + invader->w / 2;
                        float32 invTop = invader->y - invader->h / 2;
                        float32 invBottom = invader->y + invader->h / 2;

                        if (bltTop <= invBottom && bltBottom >= invTop &&
                            bltLeft <= invRight && bltRight >= invLeft) {
                            bullet->active = false;
                            invader->active = false;
                        }
                    }
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
        SDL_RenderCopy(renderer, g_textures[0], NULL, &r);
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

            int baseIndex = cInvaderTextureTable[invader->invaderType];
            int textureIndex = baseIndex + (invader->frame & 0x1);
            SDL_RenderCopy(renderer, g_textures[textureIndex], NULL, &r);
        }
    }

    for (int i = 0; i < MAX_TANK_BULLETS; ++i) {
        BulletState* bullet = &tank->bullets[i];
        if (bullet->active) {
            int hw = bullet->w / 2;
            int hh = bullet->h / 2;
            SDL_Rect r = {
                bullet->x - hw, bullet->y - hh,
                bullet->w, bullet->h,
            };
            SDL_RenderCopy(renderer, g_textures[8], NULL, &r);
        }
    }
}

void play_reset(PlayState* self) {
    tank_reset(&self->tank);

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
    self->moveDelay = g_config.invaderMaxMoveDelay;
    self->moveIndex = 0;
    for (int i = 0; i < INVADER_MOVE_QUEUE_SIZE; ++i) {
        self->moveQueue[i] = InvaderMove_Right;
    }
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
    self->w = 2;
    self->h = 6;
    self->active = false;
    self->source = bulletSource;
}

void invader_reset(InvaderState* self, int x, int y, int invaderType) {
    self->x = x;
    self->y = y;
    self->w = cInvaderWidthTable[invaderType];
    self->h = cInvaderHeightTable[invaderType];
    self->active = true;
    self->moveDelay = 0.f;
    self->invaderType = invaderType;
    self->frame = 0;
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
