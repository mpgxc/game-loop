#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <termios.h>

#define FRAME_RATE 8
#define FRAME_TIME_MS (1000 / FRAME_RATE)
#define clearScreen() system("clear")

static int size_m = 30;
char keyboard_input;

typedef struct
{
    int x;
    int y;
} Pawn;

int **createMatrix(int size)
{
    int **matrix = (int **)calloc(size, sizeof(int *));

    if (!matrix)
    {
        printf("Error allocating memory for matrix rows\n");

        return NULL;
    }

    for (int i = 0; i < size; ++i)
    {
        matrix[i] = (int *)calloc(size, sizeof(int));
        if (!matrix[i])
        {
            printf("Error allocating memory for matrix columns\n");

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

void freeMatrix(int **matrix, int size)
{
    for (int i = 0; i < size; ++i)
    {
        free(matrix[i]);
    }

    free(matrix);
}

void draw(int **matrix, int size, int random)
{
    for (int i = 0; i < size; ++i)
    {

        if (random != 0)
        {
            matrix[i][i] = rand() % 10;
            matrix[i][size - i - 1] = rand() % 10;
        }

        if (random != 1)
        {
            matrix[i][size / 2] = rand() % 10;
            matrix[size / 2][i] = rand() % 10;
        }
    }
}

void clearMatrix(int **matrix, int size)
{
    for (int i = 0; i < size; ++i)
    {
        for (int j = 0; j < size; ++j)
        {
            matrix[i][j] = 0;
        }
    }
}

void movePawn(Pawn *pawn, char input)
{
    switch (input)
    {
    case 'a':
        if (pawn->y > 0)
            pawn->y--;
        break;
    case 'd':
        if (pawn->y < size_m - 1)
            pawn->y++;
        break;
    case 'w':
        if (pawn->x > 0)
            pawn->x--;
        break;
    case 's':
        if (pawn->x < size_m - 1)
            pawn->x++;
        break;
    default:
        break;
    }
}

void renderPawn(int **matrix, int size, Pawn *pawn)
{
    if (pawn->x >= 0 && pawn->x < size && pawn->y >= 0 && pawn->y < size)
    {
        matrix[pawn->x][pawn->y] = 9;
    }
}

void update(int **matrix, int size, int *random, char keyboard_input, Pawn *pawn)
{

    clearMatrix(matrix, size);

    draw(matrix, size, *random);

    if (keyboard_input)
    {

        movePawn(pawn, keyboard_input);
        renderPawn(matrix, size, pawn);
    }

    *random = *random ^ 1;
}

void render(int **matrix, int size)
{

    for (int i = 0; i < size; ++i)
    {
        for (int j = 0; j < size; ++j)
        {

            int x = matrix[i][j];

            if (x != 0)
            {

                printf("\033[0;32m");
                printf("%d ", x);
            }
            else
            {

                printf("\033[0;37m");
                printf("%d ", x);
            }
        }

        printf("\n");
    }
}

void *listen_keyboard(void *arg)
{
    struct termios old_tio, new_tio;
    tcgetattr(STDIN_FILENO, &old_tio);
    new_tio = old_tio;
    new_tio.c_lflag &= (~ICANON & ~ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);

    while (1)
    {
        keyboard_input = getchar();
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
}

int main()
{

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, listen_keyboard, NULL);

    int **matrix = createMatrix(size_m);

    if (!matrix)
    {
        printf("Error creating matrix\n");

        return EXIT_FAILURE;
    }

    int random = 0;

    clock_t startTime, endTime;
    int elapsedTime;

    Pawn pawn = {size_m / 2, size_m / 2};

    for (;;)
    {
        startTime = clock();

        char input = keyboard_input;

        update(matrix, size_m, &random, input, &pawn);

        clearScreen();

        render(matrix, size_m);

        printf("\033[0;35m");
        printf("Command: %c\n", input);
        printf("Position: %d %d\n", pawn.x, pawn.y);

        endTime = clock();
        elapsedTime = (int)(endTime - startTime) * 1000 / CLOCKS_PER_SEC;

        if (elapsedTime < FRAME_TIME_MS)
        {
            int delayTime = FRAME_TIME_MS - elapsedTime;
            usleep(delayTime * 1000);
        }
    }

    freeMatrix(matrix, size_m);

    return EXIT_SUCCESS;
}