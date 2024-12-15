#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define GRID_SIZE 20
#define NUM_THREADS 4
#define GENERATIONS 32

int grid[GRID_SIZE][GRID_SIZE];
int next_grid[GRID_SIZE][GRID_SIZE];
pthread_barrier_t barrier;

void print_grid()
{
    system("clear");
    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            if (grid[i][j] == 1)
            {
                printf("# ");
            }
            else
            {
                printf("  ");
            }
        }
        printf("\n");
    }
    usleep(1000000);
}

int count_live(int row, int col)
{
    int count = 0;
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            if (i == 0 && j == 0)
                continue;
            int ni = row + i;
            int nj = col + j;

            if (ni >= 0 && ni < GRID_SIZE && nj >= 0 && nj < GRID_SIZE)
            {
                count += grid[ni][nj];
            }
        }
    }
    return count;
}

void *compute_next_gen(void *arg)
{
    int thread_id = (int)arg;
    int rows_per_thread = GRID_SIZE / NUM_THREADS;
    int start_row = thread_id * rows_per_thread;
    int end_row = (thread_id + 1) * rows_per_thread;

    for (int gen = 0; gen < GENERATIONS; gen++)
    {
        for (int i = start_row; i < end_row; i++)
        {
            for (int j = 0; j < GRID_SIZE; j++)
            {
                int live_neighbors = count_live(i, j);

                if (grid[i][j] == 1)
                {

                    if (live_neighbors < 2 || live_neighbors > 3)
                    {
                        next_grid[i][j] = 0;
                    }
                    else
                    {
                        next_grid[i][j] = 1;
                    }
                }
                else
                {

                    if (live_neighbors == 3)
                    {
                        next_grid[i][j] = 1;
                    }
                    else
                    {
                        next_grid[i][j] = 0;
                    }
                }
            }
        }

        pthread_barrier_wait(&barrier);

        if (thread_id == 0)
        {
            for (int i = 0; i < GRID_SIZE; i++)
            {
                for (int j = 0; j < GRID_SIZE; j++)
                {
                    grid[i][j] = next_grid[i][j];
                }
            }

            print_grid();
        }

        pthread_barrier_wait(&barrier);
    }

    return NULL;
}

void initialize_grid(int grid[GRID_SIZE][GRID_SIZE])
{
    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            grid[i][j] = 0;
        }
    }
}

void initialize_patterns(int grid[GRID_SIZE][GRID_SIZE])
{
    initialize_grid(grid);

    grid[1][1] = 1;
    grid[1][2] = 1;
    grid[2][1] = 1;
    grid[2][2] = 1;

    grid[5][6] = 1;
    grid[6][6] = 1;
    grid[7][6] = 1;

    grid[10][10] = 1;
    grid[11][11] = 1;
    grid[12][9] = 1;
    grid[12][10] = 1;
    grid[12][11] = 1;
}

int main()
{
    initialize_patterns(grid);
    pthread_barrier_init(&barrier, NULL, NUM_THREADS);

    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++)
    {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, compute_next_gen, &thread_ids[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    pthread_barrier_destroy(&barrier);
    return 0;
}