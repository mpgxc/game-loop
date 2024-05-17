
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define WIDTH 80
#define HEIGHT 24

#define clearScreen() printf("\033[H\033[J");

void drawBuffer(char buffer[HEIGHT][WIDTH])
{
    clearScreen();
    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            putchar(buffer[y][x]);
        }
        putchar('\n');
    }
}

void updateBuffer(char buffer[HEIGHT][WIDTH], int frame)
{
    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            buffer[y][x] = ' ';
        }
    }

    buffer[HEIGHT / 2][frame % WIDTH] = '@';
}

int main()
{
    char buffer[HEIGHT][WIDTH];
    int frame = 0;
    int fps = 60;
    int frameDelay = 1000 / fps;

    for (;;)
    {
        updateBuffer(buffer, frame);

        drawBuffer(buffer);

        frame += 1;

        usleep(frameDelay * 1000);
    }

    return EXIT_SUCCESS;
}
