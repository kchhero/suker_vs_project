#include "tetris.h"
#include "audio.h"

//AudioData bgm_data;
//SDL_AudioDeviceID bgm_dev = 0;

class Tetris {
    public:
        Tetris();
        ~Tetris();
        void run();

    private:
        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;

		AudioManager audio_manager;

        int board[BOARD_HEIGHT][BOARD_WIDTH]{};
        tetris current{}, next{};
        int score = 0;
        int remaining = 30;
        Uint32 last_tick = 0;

		void draw_digit(int digit, int x, int y);
		void draw_number(int number, int x, int y);
        void draw_block(int x, int y, SDL_Color color);
        void draw_board();
        void draw_tetris(const tetris& t);
        void draw_info_panel();
        void place_tetris();
        bool check_collision(const tetris& t);
        void handle_input(const SDL_Event& e);
};

Tetris::Tetris() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    window = SDL_CreateWindow("Tetris C++", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, nullptr);

    srand(static_cast<unsigned>(time(nullptr)));
    current.shape = rand() % 7 + 1;    
	current.rotation = 0;
	current.x = 3;
    current.y = 0;

    next.shape = rand() % 7 + 1;
    next.rotation = 0;
    next.x = 3;
    next.y = 0;

	audio_manager.init();
	audio_manager.play();
}

Tetris::~Tetris() {
	//audio_manager.stop();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Tetris::draw_digit(int digit, int x, int y) {
    static const int font[10][5] = {
        {0b111, 0b101, 0b101, 0b101, 0b111}, // 0
        {0b010, 0b110, 0b010, 0b010, 0b111}, // 1
        {0b111, 0b001, 0b111, 0b100, 0b111}, // 2
        {0b111, 0b001, 0b111, 0b001, 0b111}, // 3
        {0b101, 0b101, 0b111, 0b001, 0b001}, // 4
        {0b111, 0b100, 0b111, 0b001, 0b111}, // 5
        {0b111, 0b100, 0b111, 0b101, 0b111}, // 6
        {0b111, 0b001, 0b001, 0b001, 0b001}, // 7
        {0b111, 0b101, 0b111, 0b101, 0b111}, // 8
        {0b111, 0b101, 0b111, 0b001, 0b111}  // 9
    };

    for (int dy = 0; dy < 5; dy++) {
        for (int dx = 0; dx < 3; dx++) {
            if (font[digit][dy] & (1 << (2 - dx))) {
                SDL_FRect pixel = { x + dx * 4, y + dy * 4, 3, 3 };
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderFillRect(renderer, &pixel);
            }
        }
    }
}

void Tetris::draw_number(int number, int x, int y) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", number);

    int offset = 0;
    for (int i = 0; buf[i] != '\0'; i++) {
        draw_digit(buf[i] - '0', x + offset, y);
        offset += 16; // 각 숫자 사이 간격
    }
}

void Tetris::draw_block(int x, int y, SDL_Color color) {
    SDL_FRect rect = { x * BLOCK_SIZE, y * BLOCK_SIZE, BLOCK_SIZE - 1, BLOCK_SIZE - 1 };
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    SDL_RenderFillRect(renderer, &rect);
}

void Tetris::draw_board() {
    for (int y = 0; y < BOARD_HEIGHT; ++y) {
        for (int x = 0; x < BOARD_WIDTH; ++x) {
            if (board[y][x] == 0) {
				draw_block(x, y, { 22, 22, 22 });
            }
            else {
                draw_block(x, y, colors[board[y][x]]);
            }
        }
    }
    /*for (int y = 0; y < BOARD_HEIGHT; ++y)
        for (int x = 0; x < BOARD_WIDTH; ++x)
            draw_block(x, y, colors[board[y][x]]);*/
}

void Tetris::draw_tetris(const tetris& t) {
    for (int dy = 0; dy < 4; ++dy)
        for (int dx = 0; dx < 4; ++dx)
            if (blocks[t.shape][t.rotation][dy][dx])
                draw_block(t.x + dx, t.y + dy, colors[t.shape]);
}

bool Tetris::check_collision(const tetris& t) {
    for (int dy = 0; dy < 4; ++dy) {
        for (int dx = 0; dx < 4; ++dx) {
            if (blocks[t.shape][t.rotation][dy][dx]) {
                int nx = t.x + dx;
                int ny = t.y + dy;
                if (nx < 0 || nx >= BOARD_WIDTH || ny >= BOARD_HEIGHT || (ny >= 0 && board[ny][nx])) {
                    return true;
                }
            }
        }
    }
    return false;
}

void Tetris::place_tetris() {
    for (int dy = 0; dy < 4; ++dy)
        for (int dx = 0; dx < 4; ++dx)
            if (blocks[current.shape][current.rotation][dy][dx]) {
                int x = current.x + dx;
                int y = current.y + dy;
                if (y >= 0 && y < BOARD_HEIGHT && x >= 0 && x < BOARD_WIDTH)
                    board[y][x] = current.shape;
            }

    for (int y = 0; y < BOARD_HEIGHT; ++y) {
        bool full = true;
        for (int x = 0; x < BOARD_WIDTH; ++x)
            if (!board[y][x]) full = false;

        if (full) {
            for (int yy = y; yy > 0; --yy)
                for (int x = 0; x < BOARD_WIDTH; ++x)
                    board[yy][x] = board[yy - 1][x];
            score += 100;
        }
    }

    current = next;
    next.shape = rand() % 7 + 1;
    next.rotation = 0;
    next.x = 3;
    next.y = -2;
    --remaining;
}

void Tetris::draw_info_panel() {
    SDL_Color panel_color = { 22, 22, 22 };
    SDL_FRect panel = { BOARD_WIDTH * BLOCK_SIZE, 0, INFO_WIDTH * BLOCK_SIZE, SCREEN_HEIGHT };
    SDL_SetRenderDrawColor(renderer, panel_color.r, panel_color.g, panel_color.b, 255);
    SDL_RenderFillRect(renderer, &panel);

    // Draw next block
    tetris preview = next;
	preview.rotation = 0;
    preview.x = BOARD_WIDTH + 1;
    preview.y = 2;
    draw_tetris(preview);

    draw_number(score, BOARD_WIDTH * BLOCK_SIZE + 10, 150);
    draw_number(remaining, BOARD_WIDTH* BLOCK_SIZE + 10, 250);
}

void Tetris::handle_input(const SDL_Event& e) {
    if (e.type == SDL_EVENT_KEY_DOWN) {
        tetris moved = current;
        if (e.key.key == SDLK_LEFT) moved.x--;
        else if (e.key.key == SDLK_RIGHT) moved.x++;
        else if (e.key.key == SDLK_DOWN) moved.y++;
        else if (e.key.key == SDLK_UP) moved.rotation = (moved.rotation + 1) % 4;
        else if (e.key.key == SDLK_SPACE) {
            while (!check_collision(moved)) {
                current = moved;
                moved.y++;
            }
            place_tetris();
            return;
        }
        if (!check_collision(moved)) current = moved;
    }
}

void Tetris::run() {
    bool running = true;
    last_tick = SDL_GetTicks();

    while (running && remaining > 0) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
            else handle_input(e);
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
        draw_tetris(current);
        draw_info_panel();

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
}

int main() {
    Tetris game;
    game.run();
    return 0;
}