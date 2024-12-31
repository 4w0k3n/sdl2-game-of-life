#include <SDL2/SDL.h>

#include <iostream>
#include <vector>

#define COLOR_WHITE 0x444445
#define COLOR_DEAD 0x000000
#define COLOR_ALIVE 0xFFFFFF
#define CELL_SIZE 20
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define ROWS SCREEN_HEIGHT / CELL_SIZE
#define COLS SCREEN_WIDTH / CELL_SIZE
#define CELL_COUNT ROWS *COLS

class Cell {
   private:
	bool alive;

   public:
	bool getAlive() { return alive; }
	void setAlive(bool alive) { this->alive = alive; }
	Cell(bool alive) { this->alive = alive; }
	void drawCell(SDL_Surface *surface, int col, int row) {
		if (this->alive) {
			SDL_Rect rect = {col * CELL_SIZE, row * CELL_SIZE, CELL_SIZE,
							 CELL_SIZE};
			SDL_FillRect(surface, &rect, COLOR_ALIVE);
		}

		if (!this->alive) {
			SDL_Rect rect = {col * CELL_SIZE, row * CELL_SIZE, CELL_SIZE,
							 CELL_SIZE};
			SDL_FillRect(surface, &rect, COLOR_DEAD);
		}
	}
};

class Grid {
   private:
	std::vector<Cell> state;

   public:
	Grid() {
		for (int i = 0; i < CELL_COUNT; i++) {
			this->state.push_back(Cell(false));
		}
	}

	Cell &getCell(int x, int y) { return this->state.at(x + y * COLS); }

	void drawGrid(SDL_Surface *surface) {
		// Draw cells
		for (int i = 0; i < ROWS; i++) {
			for (int j = 0; j < COLS; j++) {
				this->state[i * COLS + j].drawCell(surface, j, i);
			}
		}

		// Draw vertical lines
		for (int i = 0; i <= SCREEN_WIDTH - 1; i += CELL_SIZE) {
			SDL_Rect rect = {i, 0, 1, SCREEN_HEIGHT};
			SDL_FillRect(surface, &rect, COLOR_WHITE);
		}

		// Draw horizontal lines
		for (int i = 0; i <= SCREEN_HEIGHT - 1; i += CELL_SIZE) {
			SDL_Rect rect = {0, i, SCREEN_WIDTH, 1};
			SDL_FillRect(surface, &rect, COLOR_WHITE);
		}
		// // Draw the rightmost vertical line
		SDL_Rect rightEdge = {SCREEN_WIDTH - 1, 0, 1, SCREEN_HEIGHT};
		SDL_FillRect(surface, &rightEdge, COLOR_WHITE);

		// Draw the bottommost horizontal line
		SDL_Rect bottomEdge = {0, SCREEN_HEIGHT - 1, SCREEN_WIDTH, 1};
		SDL_FillRect(surface, &bottomEdge, COLOR_WHITE);
	}
};

int main() {
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return 1;
	}
	SDL_Window *window = SDL_CreateWindow("SDL2 - GOL", SDL_WINDOWPOS_CENTERED,
										  SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH,
										  SCREEN_HEIGHT, 0);

	if (!window) {
		std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}

	bool WINDOW_OPEN = true;
	SDL_Surface *surface = SDL_GetWindowSurface(window);
	Grid grid = Grid();

	while (WINDOW_OPEN) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					WINDOW_OPEN = false;
					break;
				case SDL_MOUSEBUTTONDOWN: {
					int col = event.motion.x / CELL_SIZE;
					int row = event.motion.y / CELL_SIZE;
					Cell &cell = grid.getCell(col, row);
					cell.setAlive(!cell.getAlive());
					break;
				}
				default:
					break;
			}
		}
		grid.drawGrid(surface);
		SDL_UpdateWindowSurface(window);
		SDL_Delay(16);
	}

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}