#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20
#define BLOCK_SIZE 24
#define INFO_WIDTH 6  // 추가 공간
#define SCREEN_WIDTH ((BOARD_WIDTH + INFO_WIDTH) * BLOCK_SIZE)
#define SCREEN_HEIGHT (BOARD_HEIGHT * BLOCK_SIZE)

typedef struct {
    int x, y;
    int shape;
    int rotation;
} tetris;
int blocks[7][4][4][4] = {
    // I
    {
        {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}},
        {{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}},
        {{0,0,0,0},{0,0,0,0},{1,1,1,1},{0,0,0,0}},
        {{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0}}
    },
    // O
    {
        {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}}
    },
    // T
    {
        {{0,1,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,0,0},{0,1,1,0},{0,1,0,0},{0,0,0,0}},
        {{0,0,0,0},{1,1,1,0},{0,1,0,0},{0,0,0,0}},
        {{0,1,0,0},{1,1,0,0},{0,1,0,0},{0,0,0,0}}
    },
    // L
    {
        {{0,0,1,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,0,0},{0,1,0,0},{0,1,1,0},{0,0,0,0}},
        {{0,0,0,0},{1,1,1,0},{1,0,0,0},{0,0,0,0}},
        {{1,1,0,0},{0,1,0,0},{0,1,0,0},{0,0,0,0}}
    },
    // J
    {
        {{1,0,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,1,0},{0,1,0,0},{0,1,0,0},{0,0,0,0}},
        {{0,0,0,0},{1,1,1,0},{0,0,1,0},{0,0,0,0}},
        {{0,1,0,0},{0,1,0,0},{1,1,0,0},{0,0,0,0}}
    },
    // S
    {
        {{0,1,1,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,0,0},{0,1,1,0},{0,0,1,0},{0,0,0,0}},
        {{0,0,0,0},{0,1,1,0},{1,1,0,0},{0,0,0,0}},
        {{1,0,0,0},{1,1,0,0},{0,1,0,0},{0,0,0,0}}
    },
    // Z
    {
        {{1,1,0,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,0,1,0},{0,1,1,0},{0,1,0,0},{0,0,0,0}},
        {{0,0,0,0},{1,1,0,0},{0,1,1,0},{0,0,0,0}},
        {{0,1,0,0},{1,1,0,0},{1,0,0,0},{0,0,0,0}}
    }
};

int board[BOARD_HEIGHT][BOARD_WIDTH] = { 0 };
tetris current, next;
int score = 0;
int remaining = 30;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_AudioDeviceID audio_device = 0;
SDL_AudioSpec audio_spec;
Uint8* audio_buf = NULL;
Uint32 audio_len = 0;

void draw_block(int x, int y, SDL_Color color) {
    SDL_FRect rect = { x * BLOCK_SIZE, y * BLOCK_SIZE, BLOCK_SIZE - 1, BLOCK_SIZE - 1 };
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    SDL_RenderFillRect(renderer, &rect);
}

void draw_board() {
    SDL_Color colors[] = {
        {0,0,0}, {255,0,0}, {0,255,0}, {0,0,255},
        {255,255,0}, {255,0,255}, {0,255,255}, {255,128,0}
    };
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            draw_block(x, y, colors[board[y][x]]);
        }
    }
}

void draw_tetris(tetris t, SDL_Color color) {
    for (int dy = 0; dy < 4; dy++) {
        for (int dx = 0; dx < 4; dx++) {
            if (blocks[t.shape][t.rotation][dy][dx]) {
                draw_block(t.x + dx, t.y + dy, color);
            }
        }
    }
}

int check_collision(tetris t) {
    for (int dy = 0; dy < 4; dy++) {
        for (int dx = 0; dx < 4; dx++) {
            if (blocks[t.shape][t.rotation][dy][dx]) {
                int nx = t.x + dx;
                int ny = t.y + dy;
                if (nx < 0 || nx >= BOARD_WIDTH || ny >= BOARD_HEIGHT || (ny >= 0 && board[ny][nx])) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

void place_tetris() {
    for (int dy = 0; dy < 4; dy++) {
        for (int dx = 0; dx < 4; dx++) {
            if (blocks[current.shape][current.rotation][dy][dx]) {
                int x = current.x + dx;
                int y = current.y + dy;
                if (y >= 0 && y < BOARD_HEIGHT && x >= 0 && x < BOARD_WIDTH) {
                    board[y][x] = current.shape + 1;
                }
            }
        }
    }

    // 사운드 재생
    /*SDL_LockAudioDevice(audio_device);
    SDL_QueueAudio(audio_device, audio_buf, audio_len);
    SDL_PauseAudioDevice(audio_device, 0);*/

    // 라인 제거
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        int full = 1;
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (!board[y][x]) full = 0;
        }
        if (full) {
            for (int yy = y; yy > 0; yy--) {
                for (int x = 0; x < BOARD_WIDTH; x++) {
                    board[yy][x] = board[yy - 1][x];
                }
            }
            score += 100;
        }
    }

    current = next;
    next.shape = rand() % 7;
    next.rotation = 0;
    next.x = 3;
    next.y = -2;
    remaining--;
}

void draw_info_panel() {
    SDL_Color text_color = { 255, 255, 255 };
    SDL_Color bg_color = { 32, 32, 32 };
    SDL_Color _temp_color = { 200, 200, 200, 0 };

    // Info Panel 배경
    SDL_FRect panel = { BOARD_WIDTH * BLOCK_SIZE, 0, INFO_WIDTH * BLOCK_SIZE, SCREEN_HEIGHT };
    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, 255);
    SDL_RenderFillRect(renderer, &panel);

    // 다음 블럭
    tetris preview = next;
    preview.x = BOARD_WIDTH + 1;
    preview.y = 2;
    draw_tetris(preview, _temp_color);

    // 점수 및 남은 블럭은 간단히 사각형 칸으로 표현 (폰트 없으므로 생략)
    for (int i = 0; i < score / 100; i++) {
        SDL_FRect s = { BOARD_WIDTH * BLOCK_SIZE + 4, 150 + i * 10, 10, 8 };
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        SDL_RenderFillRect(renderer, &s);
    }

    for (int i = 0; i < remaining; i++) {
        SDL_FRect r = { BOARD_WIDTH * BLOCK_SIZE + 20, 300 + i * 3, 6, 2 };
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &r);
    }
}

int main() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    window = SDL_CreateWindow("TetrisSimple", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, NULL);

    //SDL_LoadWAV("drop.wav", &audio_spec, &audio_buf, &audio_len);
    //audio_device = SDL_OpenAudioDevice(NULL, 0, &audio_spec, NULL, 0);

    srand((unsigned int)time(NULL));
    current.shape = rand() % 7;
    current.rotation = 0;
    current.x = 3;
    current.y = -2;
    next.shape = rand() % 7;
    next.rotation = 0;
    next.x = 3;
    next.y = -2;

    Uint32 last_tick = SDL_GetTicks();
    int running = 1;

    while (running && remaining > 0) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = 0;
            if (e.type == SDL_EVENT_KEY_DOWN) {
                tetris moved = current;
                if (e.key.key == SDLK_LEFT) moved.x--;
                if (e.key.key == SDLK_RIGHT) moved.x++;
                if (e.key.key == SDLK_DOWN) moved.y++;
                if (e.key.key == SDLK_UP) moved.rotation = (moved.rotation + 1) % 4;
                if (e.key.key == SDLK_SPACE) {
                    while (!check_collision(moved)) {
                        current = moved;
                        moved.y++;
                    }
                    place_tetris();
                }
                else if (!check_collision(moved)) {
                    current = moved;
                }
            }
        }

        Uint32 now = SDL_GetTicks();
        if (now - last_tick > 500) {
            tetris moved = current;
            moved.y++;
            if (!check_collision(moved)) {
                current = moved;
            }
            else {
                place_tetris();
            }
            last_tick = now;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        draw_board();
        SDL_Color _temp_color = { 255, 255, 255, 0 };
        draw_tetris(current, _temp_color);
        draw_info_panel();
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_CloseAudioDevice(audio_device);
    //SDL_FreeWAV(audio_buf);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}