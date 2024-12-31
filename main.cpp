#include <SDL2/SDL.h>

#include <iostream>
#include <vector>

#define COLOR_WHITE 0x444445
#define COLOR_DEAD 0x000000
#define COLOR_ALIVE 0xFFFFFF
#define CELL_SIZE 8
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
	int generation_count;

   public:
	Grid() {
		for (int i = 0; i < CELL_COUNT; i++) {
			this->state.push_back(Cell(false));
		}
		this->generation_count = 0;
	}

	Cell &getCell(int x, int y) { return this->state.at(x + y * COLS); }
	int getGeneration() { return this->generation_count; }

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

	int getAliveNeighboursCount(int x, int y) {
		std::vector<int> x_neighbours = {-1, 0, 1, -1, 1, -1, 0, 1};
		std::vector<int> y_neighbours = {-1, -1, -1, 0, 0, 1, 1, 1};
		int result = 0;
		for (int i = 0; i < 8; i++) {
			int n_x = x + x_neighbours[i];
			int n_y = y + y_neighbours[i];

			if (n_x < 0) n_x = COLS - 1;
			if (n_x >= COLS) n_x = 0;
			if (n_y < 0) n_y = ROWS - 1;
			if (n_y >= ROWS) n_y = 0;

			Cell &cell = getCell(n_x, n_y);
			if (cell.getAlive()) result++;
		}
		return result;
	}

	void mutateNextGeneration() {
		std::vector<Cell> result =
			this->state;				  // Create a copy of the current state
		for (int i = 0; i < ROWS; i++) {  // Iterate over rows
			for (int y = 0; y < COLS; y++) {  // Iterate over columns
				int neighbour_count = this->getAliveNeighboursCount(y, i);
				bool isAlive = this->getCell(y, i).getAlive();
				int index = i * COLS + y;  // Correct index formula

				if (isAlive) {
					if (neighbour_count < 2 || neighbour_count > 3) {
						result.at(index).setAlive(false);  // Update cell state
					}
				} else {
					if (neighbour_count == 3) {
						result.at(index).setAlive(true);  // Update cell state
					}
				}
			}
		}
		this->state = std::move(result);  // Update the current state
		this->generation_count++;
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
		bool SIM_RUNNING = false;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					WINDOW_OPEN = false;
					break;
				case SDL_MOUSEMOTION: {
					if (event.motion.state) {
						int col = event.motion.x / CELL_SIZE;
						int row = event.motion.y / CELL_SIZE;
						std::cout << "x:" << col << std::endl;
						std::cout << "y:" << row << std::endl;
						Cell &cell = grid.getCell(col, row);
						std::cout << "neighbours alive:"
								  << grid.getAliveNeighboursCount(col, row)
								  << std::endl;
						if (event.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT))
							cell.setAlive(true);
						if (event.motion.state & SDL_BUTTON(SDL_BUTTON_RIGHT))
							cell.setAlive(false);
						break;
					}
				}
				case SDL_KEYDOWN: {
					if (event.key.keysym.sym == SDLK_SPACE) {
						SIM_RUNNING = !SIM_RUNNING;
						std::cout << "sim "
								  << (SIM_RUNNING ? "running" : "paused")
								  << std::endl;
					}
				}
				default:
					break;
			}
		}
		if (SIM_RUNNING) {
			std::cout << "current gen:" << grid.getGeneration() << std::endl;
			grid.mutateNextGeneration();
		}
		grid.drawGrid(surface);
		SDL_UpdateWindowSurface(window);
		SDL_Delay(20);
	}

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}