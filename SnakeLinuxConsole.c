#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>

typedef struct snakePart {
    int x;
    int y;
    struct snakePart *next;
} snakePart;

typedef struct food {
    int x;
    int y;
} food;

typedef enum Direction { UP, DOWN, LEFT, RIGHT } Direction;

typedef struct v2d{
    int x;
    int y;
} v2d;

void snake_push_front(snakePart** snake, v2d coord)
{
    snakePart *part = malloc(sizeof(snakePart));
    part->x = coord.x;
    part->y = coord.y;
    part->next = *snake;
    *snake = part;
}

void snake_pop_end(snakePart* snake)
{
    snakePart *pneultimate = snake;
    
    while (pneultimate->next->next != NULL)
    {
        pneultimate = pneultimate->next;
    }

    pneultimate->next = NULL;
    free(pneultimate->next);
}

snakePart* snake_last(snakePart* snake)
{
    snakePart *last = snake;
    
    while (last->next != NULL)
    {
        last = last->next;
    }

    return last;
}

void snake_push_end(snakePart* snake, v2d coord)
{
    snakePart *last = snake_last(snake);
    
    snakePart *part = malloc(sizeof(snakePart));
    part->x = coord.x;
    part->y = coord.y;
    last->next = part;
}

int main()
{
    int screenHeight = 30;
    int screenWidth = 60;

    v2d startCoords[5] = { {30, 5}, {31, 5}, {32, 5}, {33, 5}, {34, 5} };

    snakePart *snake = malloc(sizeof(snakePart));
    for(int i=0; i < (sizeof(startCoords) / sizeof(v2d)); i++) {
        snake_push_front(&snake, startCoords[i]);
    }

    char screen_buffer[1024];
    setvbuf(stdout, screen_buffer, _IOFBF, sizeof(screen_buffer));
    static struct termios old_attr, new_attr;

    // Save the current terminal attributes
    tcgetattr(STDIN_FILENO, &old_attr);
    new_attr = old_attr;

    // Disable canonical mode and echo
    new_attr.c_lflag &= ~(ICANON | ECHO);
    new_attr.c_cc[VMIN] = 0;  // Minimum number of characters for non-blocking read
    new_attr.c_cc[VTIME] = 1; // Timeout in deciseconds (0 = no timeout)
    tcsetattr(STDIN_FILENO, TCSANOW, &new_attr);

    Direction snakeDirection = RIGHT;
    Direction oldSnakeDirection = snakeDirection;

    food food = { 10, 5 };

    int score = 0;

    while (1) {
        
        char keyPressed;
        usleep(20000);

        if (read(STDIN_FILENO, &keyPressed, 1) == 1) {
            if (keyPressed == '\033') {
                char seq[2];
                read(STDIN_FILENO, seq, 2); // Read next two bytes
                if (seq[0] == '[') {
                    if (seq[1] == 'A' && oldSnakeDirection != DOWN) {
                        snakeDirection = UP;
                    }
                    if (seq[1] == 'B' && oldSnakeDirection != UP) {
                        snakeDirection = DOWN;
                    }
                    if (seq[1] == 'C' && oldSnakeDirection != LEFT) {
                        snakeDirection = RIGHT;
                    }
                    if (seq[1] == 'D' && oldSnakeDirection != RIGHT) {
                        snakeDirection = LEFT;
                    }
                    oldSnakeDirection = snakeDirection;
                }
            }
        }

        switch (snakeDirection) {
            case UP:
                snake_push_front(&snake, (v2d){ snake->x, snake->y - 1 });
                break;
            case DOWN:
                snake_push_front(&snake, (v2d){ snake->x, snake->y + 1 });
                break;
            case LEFT:
                snake_push_front(&snake, (v2d){ snake->x -1, snake->y });
                break;
            case RIGHT:
                snake_push_front(&snake, (v2d){ snake->x + 1, snake->y });
                break;
        }

        // Remove snake tail
        snake_pop_end(snake);

        // Detection
        if (snake->x == food.x && snake->y == food.y) {
            score++;

            food.x = rand() % screenWidth;
            food.y = rand() % screenHeight;

            snakePart *last;
            for (int i = 0; i < 3; i++) {
                last = snake_last(snake);
                snake_push_end(snake, (v2d){ last->x, last->y });
            }
        }

        // Draw
        printf("\033[2J");
        printf("\033[H");

        char line[screenWidth];
        for (int i = 0; i < screenWidth; i++) {            
             line[i] = '-';
        }
        printf("score:%d%s\n", score, line);

        printf("\033[%d;%dHO", snake->y, snake->x);
        snakePart *part = snake;
        while ((part = part->next) && part->next != NULL) {
            printf("\033[%d;%dHo", part->y, part->x);
        }

        printf("\033[%d;%dH@", food.y, food.x);
        fflush(stdout);
    }
    return 0;
}
