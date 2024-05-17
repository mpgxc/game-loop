#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <termios.h>
#include <string.h>

#define FRAME_RATE 8
#define FRAME_TIME_MS (1000 / FRAME_RATE)
#define clearScreen() printf("\033[H\033[J")

static int matrix_size = 30;
char keyboard_input;

typedef struct
{
    int x;
    int y;
} Pawn;

char **createMatrix(int size)
{
    char **matrix = (char **)calloc(size, sizeof(char *));
    if (!matrix)
    {
        fprintf(stderr, "Error allocating memory for matrix rows\n");
        return NULL;
    }

    for (int i = 0; i < size; ++i)
    {
        matrix[i] = (char *)calloc(size, sizeof(char));
        if (!matrix[i])
        {
            fprintf(stderr, "Error allocating memory for matrix columns\n");
            for (int j = 0; j < i; j++)
            {
                free(matrix[j]);
            }
            free(matrix);
            return NULL;
        }
    }
    return matrix;
}

void freeMatrix(char **matrix, int size)
{
    for (int i = 0; i < size; ++i)
    {
        free(matrix[i]);
    }
    free(matrix);
}

void clearMatrix(char **matrix, int size)
{
    for (int i = 0; i < size; ++i)
    {
        for (int j = 0; j < size; ++j)
        {
            matrix[i][j] = ' ';
        }
    }
}

void drawBoundaries(char **matrix, int size)
{
    for (int i = 0; i < size; ++i)
    {
        matrix[0][i] = '#';
        matrix[size - 1][i] = '#';
        matrix[i][0] = '#';
        matrix[i][size - 1] = '#';
    }
}

void drawRandomPatterns(char **matrix, int size, int patternType)
{
    for (int i = 0; i < size; ++i)
    {
        if (patternType != 0)
        {
            matrix[i][i] = '0' + rand() % 10;
            matrix[i][size - i - 1] = '0' + rand() % 10;
        }
        if (patternType != 1)
        {
            matrix[i][size / 2] = '0' + rand() % 10;
            matrix[size / 2][i] = '0' + rand() % 10;
        }
    }
}

void renderMatrix(char **matrix, int size, Pawn *pawn)
{
    for (int i = 0; i < size; ++i)
    {
        for (int j = 0; j < size; ++j)
        {
            if (pawn->x == i && pawn->y == j)
            {
                printf("\033[0;33m%c ", matrix[i][j]);
            }
            else
            {

                char cell = matrix[i][j];
                if (cell == ' ')
                {
                    printf("\033[0;31m. ");
                }
                else
                {
                    printf("\033[0;32m%c ", cell);
                }
            }
        }
        printf("\n");
    }
}

void movePawn(Pawn *pawn, char input)
{
    switch (input)
    {
    case 'a':
        if (pawn->y > 0)
        {
            pawn->y--;
        }

        break;
    case 'd':
        if (pawn->y < matrix_size - 1)
        {
            pawn->y++;
        }
        break;
    case 'w':
        if (pawn->x > 0)
        {
            pawn->x--;
        }
        break;
    case 's':
        if (pawn->x < matrix_size - 1)
        {
            pawn->x++;
        }
        break;
    default:
        break;
    }
}

void renderPawn(char **matrix, int size, Pawn *pawn)
{
    if (pawn->x >= 0 && pawn->x < size && pawn->y >= 0 && pawn->y < size)
    {
        matrix[pawn->x][pawn->y] = '*';
    }
}

void *updatePawn(void *arg)
{
    Pawn *pawn = (Pawn *)arg;
    while (1)
    {
        movePawn(pawn, keyboard_input);
        usleep((1000 / 4) * 1000); // Update at 16 FPS
    }
    return NULL;
}

void *listenKeyboard(void *arg)
{
    struct termios old_tio, new_tio;
    tcgetattr(STDIN_FILENO, &old_tio);
    new_tio = old_tio;
    new_tio.c_lflag &= (~ICANON & ~ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);

    for (;;)
    {
        keyboard_input = getchar();
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
    return NULL;
}

void updateMatrix(char **matrix, int size, int *patternType)
{
    clearMatrix(matrix, size);
    drawBoundaries(matrix, size);
    drawRandomPatterns(matrix, size, *patternType);
    *patternType = *patternType ^ 1;
}

void renderScene(char **matrix, int size, Pawn *pawn)
{
    renderMatrix(matrix, size, pawn);
    printf("\033[0;35mCommand: %c\n", keyboard_input);
    printf("Position: %d %d\n", pawn->x, pawn->y);
}

int main()
{
    pthread_t keyboard_thread, pawn_thread;

    if (pthread_create(&keyboard_thread, NULL, listenKeyboard, NULL))
    {
        fprintf(stderr, "Error creating keyboard thread\n");
        return EXIT_FAILURE;
    }

    char **matrix = createMatrix(matrix_size);

    if (!matrix)
    {
        fprintf(stderr, "Error creating matrix\n");
        return EXIT_FAILURE;
    }

    int patternType = 0;

    Pawn pawn = {matrix_size / 2, matrix_size / 2};

    if (pthread_create(&pawn_thread, NULL, updatePawn, &pawn))
    {
        fprintf(stderr, "Error creating pawn thread\n");
        freeMatrix(matrix, matrix_size);
        return EXIT_FAILURE;
    }

    for (;;)
    {
        clock_t startTime = clock();

        updateMatrix(matrix, matrix_size, &patternType);
        renderPawn(matrix, matrix_size, &pawn);

        clearScreen();
        renderScene(matrix, matrix_size, &pawn);

        clock_t endTime = clock();

        int elapsedTime = (int)(endTime - startTime) * 1000 / CLOCKS_PER_SEC;

        if (elapsedTime < FRAME_TIME_MS)
        {
            usleep((FRAME_TIME_MS - elapsedTime) * 1000);
        }
    }

    freeMatrix(matrix, matrix_size);

    return EXIT_SUCCESS;
}
