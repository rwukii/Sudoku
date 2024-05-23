#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <ctime>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

const int GRID_SIZE = 9;
const int CELL_SIZE = 50;
const int PADDING = 10;

bool init(SDL_Window** window, SDL_Renderer** renderer, TTF_Font** font) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    if (TTF_Init() == -1) {
        std::cout << "TTF could not initialize! TTF_Error: " << TTF_GetError() << std::endl;
        return false;
    }

    *window = SDL_CreateWindow("Sudoku",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN);

    if (*window == nullptr) {
        std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (*renderer == nullptr) {
        std::cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    *font = TTF_OpenFont("fonts/arial.ttf", 24);
    if (*font == nullptr) {
        std::cout << "Failed to load font! TTF_Error: " << TTF_GetError() << std::endl;
        return false;
    }

    return true;
}

void close(SDL_Window* window, SDL_Renderer* renderer, TTF_Font* font) {
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

void drawGrid(SDL_Renderer* renderer, int selectedRow, int selectedCol, const std::vector<std::vector<int>>& board, const std::vector<std::vector<int>>& solvedBoard, bool checkMode) {
    for (int i = 0; i <= GRID_SIZE; ++i) {
        int thickness = (i % 3 == 0) ? 3 : 1;

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

        // Draw vertical lines
        SDL_Rect vLine = { PADDING + i * CELL_SIZE, PADDING, thickness, GRID_SIZE * CELL_SIZE };
        SDL_RenderFillRect(renderer, &vLine);

        // Draw horizontal lines
        SDL_Rect hLine = { PADDING, PADDING + i * CELL_SIZE, GRID_SIZE * CELL_SIZE, thickness };
        SDL_RenderFillRect(renderer, &hLine);
    }

    // Highlight the selected cell with reduced size
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 100);
    SDL_Rect highlight = { PADDING + selectedCol * CELL_SIZE + 2, PADDING + selectedRow * CELL_SIZE + 2, CELL_SIZE - 4, CELL_SIZE - 4 };
    SDL_RenderFillRect(renderer, &highlight);

    if (checkMode) {
        // Highlight correct or incorrect cells with reduced size
        for (int row = 0; row < GRID_SIZE; ++row) {
            for (int col = 0; col < GRID_SIZE; ++col) {
                if (board[row][col] != 0 && board[row][col] != solvedBoard[row][col]) {
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 100);
                    SDL_Rect incorrect = { PADDING + col * CELL_SIZE + 2, PADDING + row * CELL_SIZE + 2, CELL_SIZE - 4, CELL_SIZE - 4 };
                    SDL_RenderFillRect(renderer, &incorrect);
                } else if (board[row][col] == solvedBoard[row][col] && board[row][col] != 0) {
                    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 100);
                    SDL_Rect correct = { PADDING + col * CELL_SIZE + 2, PADDING + row * CELL_SIZE + 2, CELL_SIZE - 4, CELL_SIZE - 4 };
                    SDL_RenderFillRect(renderer, &correct);
                }
            }
        }
    }
}

void drawNumbers(SDL_Renderer* renderer, TTF_Font* font, const std::vector<std::vector<int>>& board) {
    SDL_Color textColor = { 0, 0, 0, 255 };

    for (int row = 0; row < GRID_SIZE; ++row) {
        for (int col = 0; col < GRID_SIZE; ++col) {
            if (board[row][col] != 0) {
                std::string num = std::to_string(board[row][col]);
                SDL_Surface* textSurface = TTF_RenderText_Solid(font, num.c_str(), textColor);
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

                int textWidth = textSurface->w;
                int textHeight = textSurface->h;
                SDL_Rect renderQuad = { PADDING + col * CELL_SIZE + (CELL_SIZE - textWidth) / 2,
                                        PADDING + row * CELL_SIZE + (CELL_SIZE - textHeight) / 2,
                                        textWidth, textHeight };

                SDL_RenderCopy(renderer, textTexture, nullptr, &renderQuad);

                SDL_FreeSurface(textSurface);
                SDL_DestroyTexture(textTexture);
            }
        }
    }
}

bool isValidMove(const std::vector<std::vector<int>>& board, int row, int col, int num) {
    for (int x = 0; x < GRID_SIZE; ++x) {
        if (board[row][x] == num || board[x][col] == num) {
            return false;
        }
    }

    int startRow = row / 3 * 3;
    int startCol = col / 3 * 3;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (board[startRow + i][startCol + j] == num) {
                return false;
            }
        }
    }

    return true;
}

bool solveSudoku(std::vector<std::vector<int>>& board, int row, int col) {
    if (row == GRID_SIZE - 1 && col == GRID_SIZE) {
        return true;
    }
    if (col == GRID_SIZE) {
        row++;
        col = 0;
    }
    if (board[row][col] != 0) {
        return solveSudoku(board, row, col + 1);
    }

    std::vector<int> numbers(GRID_SIZE);
    std::iota(numbers.begin(), numbers.end(), 1);
    std::shuffle(numbers.begin(), numbers.end(), std::mt19937{ std::random_device{}() });

    for (int num : numbers) {
        if (isValidMove(board, row, col, num)) {
            board[row][col] = num;
            if (solveSudoku(board, row, col + 1)) {
                return true;
            }
            board[row][col] = 0;
        }
    }
    return false;
}

void fillDiagonal(std::vector<std::vector<int>>& board) {
    for (int i = 0; i < GRID_SIZE; i += 3) {
        std::vector<int> numbers(GRID_SIZE);
        std::iota(numbers.begin(), numbers.end(), 1);
        std::shuffle(numbers.begin(), numbers.end(), std::mt19937{ std::random_device{}() });

        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 3; ++col) {
                board[i + row][i + col] = numbers[row * 3 + col];
            }
        }
    }
}

void removeCells(std::vector<std::vector<int>>& board, int cellsToRemove) {
    std::vector<std::pair<int, int>> cells;
    for (int row = 0; row < GRID_SIZE; ++row) {
        for (int col = 0; col < GRID_SIZE; ++col) {
            cells.emplace_back(row, col);
        }
    }
    std::shuffle(cells.begin(), cells.end(), std::mt19937{ std::random_device{}() });
    for (int i = 0; i < cellsToRemove; ++i) {
        board[cells[i].first][cells[i].second] = 0;
    }
}

std::vector<std::vector<int>> generateSudoku() {
    std::vector<std::vector<int>> board(GRID_SIZE, std::vector<int>(GRID_SIZE, 0));

    fillDiagonal(board);
    solveSudoku(board, 0, 0);
    removeCells(board, GRID_SIZE * GRID_SIZE - 30);

    return board;
}

bool isBoardComplete(const std::vector<std::vector<int>>& board) {
    for (const auto& row : board) {
        if (std::find(row.begin(), row.end(), 0) != row.end()) {
            return false;
        }
    }
    return true;
}

void drawCompletionScreen(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    SDL_Color textColor = { 0, 255, 0, 255 };
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, "Congratulations! You completed the puzzle!", textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    int textWidth = textSurface->w;
    int textHeight = textSurface->h;
    SDL_Rect renderQuad = { (SCREEN_WIDTH - textWidth) / 2, (SCREEN_HEIGHT - textHeight) / 2, textWidth, textHeight };

    SDL_RenderCopy(renderer, textTexture, nullptr, &renderQuad);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);

    SDL_RenderPresent(renderer);
    SDL_Delay(5000);
}

void drawButton(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y, int w, int h) {
    SDL_Rect buttonRect = { x, y, w, h };
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_RenderFillRect(renderer, &buttonRect);

    SDL_Color textColor = { 255, 255, 255, 255 };
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, text.c_str(), textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    int textWidth = textSurface->w;
    int textHeight = textSurface->h;
    SDL_Rect renderQuad = { x + (w - textWidth) / 2, y + (h - textHeight) / 2, textWidth, textHeight };

    SDL_RenderCopy(renderer, textTexture, nullptr, &renderQuad);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

bool isCorrect(const std::vector<std::vector<int>>& board, const std::vector<std::vector<int>>& solvedBoard, int row, int col) {
    return board[row][col] == solvedBoard[row][col];
}

void handleInput(SDL_Event& e, std::vector<std::vector<int>>& board, const std::vector<std::vector<int>>& initialBoard, const std::vector<std::vector<int>>& solvedBoard, int& selectedRow, int& selectedCol, bool& checkMode, Uint32& checkTime) {
    if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
            case SDLK_UP:
                selectedRow = (selectedRow - 1 + GRID_SIZE) % GRID_SIZE;
                break;
            case SDLK_DOWN:
                selectedRow = (selectedRow + 1) % GRID_SIZE;
                break;
            case SDLK_LEFT:
                selectedCol = (selectedCol - 1 + GRID_SIZE) % GRID_SIZE;
                break;
            case SDLK_RIGHT:
                selectedCol = (selectedCol + 1) % GRID_SIZE;
                break;
            case SDLK_1: case SDLK_2: case SDLK_3: case SDLK_4: case SDLK_5:
            case SDLK_6: case SDLK_7: case SDLK_8: case SDLK_9:
            {
                int num = e.key.keysym.sym - SDLK_0;
                if (initialBoard[selectedRow][selectedCol] == 0 && isValidMove(board, selectedRow, selectedCol, num)) {
                    board[selectedRow][selectedCol] = num;
                }
                break;
            }
            case SDLK_BACKSPACE:
            case SDLK_DELETE:
                if (initialBoard[selectedRow][selectedCol] == 0) {
                    board[selectedRow][selectedCol] = 0;
                }
                break;
            case SDLK_RETURN:
                checkMode = true;
                checkTime = SDL_GetTicks();
                break;
        }
    } else if (e.type == SDL_MOUSEBUTTONDOWN) {
        int mouseX = e.button.x;
        int mouseY = e.button.y;
        if (mouseX >= PADDING && mouseX <= PADDING + GRID_SIZE * CELL_SIZE && mouseY >= PADDING && mouseY <= PADDING + GRID_SIZE * CELL_SIZE) {
            selectedCol = (mouseX - PADDING) / CELL_SIZE;
            selectedRow = (mouseY - PADDING) / CELL_SIZE;
        } else if (mouseX >= SCREEN_WIDTH - 100 && mouseX <= SCREEN_WIDTH - 20 && mouseY >= SCREEN_HEIGHT - 50 && mouseY <= SCREEN_HEIGHT - 20) {
            checkMode = true;
            checkTime = SDL_GetTicks();
        }
    }
}

int main(int argc, char* args[]) {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    TTF_Font* font = nullptr;

    if (!init(&window, &renderer, &font)) {
        std::cout << "Failed to initialize!" << std::endl;
        return -1;
    }

    std::vector<std::vector<int>> board = generateSudoku();
    std::vector<std::vector<int>> initialBoard = board; // Copy the initial state of the board

    std::vector<std::vector<int>> solvedBoard = board;
    solveSudoku(solvedBoard, 0, 0); // Solve the board to get the solution

    int selectedRow = 0;
    int selectedCol = 0;
    bool checkMode = false;
    Uint32 checkTime = 0;

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else {
                handleInput(e, board, initialBoard, solvedBoard, selectedRow, selectedCol, checkMode, checkTime);
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        drawGrid(renderer, selectedRow, selectedCol, board, solvedBoard, checkMode);
        drawNumbers(renderer, font, board);

        // Draw the "Check" button
        drawButton(renderer, font, "Check", SCREEN_WIDTH - 100, SCREEN_HEIGHT - 50, 80, 30);

        SDL_RenderPresent(renderer);

        if (checkMode) {
            Uint32 currentTime = SDL_GetTicks();
            if (currentTime - checkTime >= 2000) {
                checkMode = false;
                if (isBoardComplete(board)) {
                    drawCompletionScreen(renderer, font);
                    quit = true;
                }
            }
        }
    }

    close(window, renderer, font);

    return 0;
}

