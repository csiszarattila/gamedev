#include <iostream>
#include <list>
#include <chrono>
#include <Windows.h>
#include <thread>

using namespace std;

struct snakePart {
    int x;
    int y;
};

enum Direction { UP, DOWN, LEFT, RIGHT };

int main()
{
    HANDLE hConsoleBuffer = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);

    SetConsoleActiveScreenBuffer(hConsoleBuffer);
    SetConsoleOutputCP(CP_UTF8);

    int screenHeight = 30;
    int screenWidth = 120;

    DWORD numOfBytesWritten = 0;

    list<snakePart> snake = { {40, 15}, {41, 15}, {42, 15}, {43, 15}, {44, 15}, {45, 15} };

    wchar_t* screen = new wchar_t[screenHeight * screenWidth];

    Direction snakeDirection = RIGHT;
    Direction oldSnakeDirection = snakeDirection;

    int foodX = 40;
    int foodY = 10;

    int score = 0;

    while (1) {
        
        auto lstFrameTime = chrono::system_clock::now();

        // Handle keys
        while ((chrono::system_clock::now() - lstFrameTime) < 200ms) {
            if (GetAsyncKeyState(VK_UP) & 0x8000 && oldSnakeDirection != DOWN) {
                snakeDirection = UP;
            }
            if (GetAsyncKeyState(VK_DOWN) & 0x8000 && oldSnakeDirection != UP) {
                snakeDirection = DOWN;
            }
            if (GetAsyncKeyState(VK_LEFT) & 0x8000 && oldSnakeDirection != RIGHT) {
                snakeDirection = LEFT;
            }
            if (GetAsyncKeyState(VK_RIGHT) & 0x8000 && oldSnakeDirection != LEFT) {
                snakeDirection = RIGHT;
            }

            oldSnakeDirection = snakeDirection;
        }

        switch (snakeDirection) {
            case UP:
                snake.push_front({ snake.front().x, snake.front().y - 1 });
                break;
            case DOWN:
                snake.push_front({ snake.front().x, snake.front().y + 1 });
                break;
            case LEFT:
                snake.push_front({ snake.front().x - 1, snake.front().y });
                break;
            case RIGHT:
                snake.push_front({ snake.front().x + 1, snake.front().y });
                break;
        }

        snake.pop_back();

        // Detection
        if (snake.front().x == foodX && snake.front().y == foodY) {
            score++;

            foodX = rand() % screenWidth;
            foodY = rand() % screenHeight;

            for (int i = 0; i < 3; i++) {
                snake.push_back({ snake.back().x, snake.back().y });
            }
        }

        // Draw
        for (int i = 0; i < screenHeight * screenWidth; i++) {
            screen[i] = L' ';
        }

        for (int i = 0; i < screenWidth; i++) {
            screen[i] = L'=';
            screen[i + screenWidth*2] = L'=';
        }
        wsprintf(&screen[screenWidth], L"Score: %d", score);


        for (auto s : snake) {
            screen[s.y * screenWidth + s.x] = L'o';
        }

        screen[snake.front().y * screenWidth + snake.front().x] = L'@';

        screen[foodY * screenWidth + foodX] = L'X';

        WriteConsoleOutputCharacter(hConsoleBuffer, screen, screenHeight * screenWidth, { 0, 0 }, &numOfBytesWritten);
    }
}
