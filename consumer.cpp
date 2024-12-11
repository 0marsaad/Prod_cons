// #include "shared.h"
// /*consumer takes in as an arguent the size of the buffer only and prints*/
// /* outputs a table in the form
// +------------+------------+------------+
// | Commodity  | Price      | AVGPrice   |
// +------------+------------+------------+
// | Aluminum   | 1000.00 ↓  | 1000.00 ↓  |
// | Silver     | 500.00 ↑   | 500.00 ↑   |
// | Copper     | 0.0        | 0.0        |
// | Gold       | 0.0        | 0.0        |
// | CrudeOil   | 0.0        | 0.0        |
// | NaturalGas | 0.0        | 0.0        |
// | Nickel     | 300.00 ↑   | 301.0 ↑    |
// | Zinc       | 0.0        | 0.0        |
// | Lead       | 0.0        | 0.0        |
// | Methaoil   | 0.0        | 0.0        |
// | Cotton     | 0.0        | 0.0        |
// +------------+------------+------------+

// build with gcc -o consumer consumer.c -Wall -Wextra -std=c99
// run with ./consumer <buffer_size>
// example: ./consumer 40

#include "shared.h"


void print_table(SharedBuffer *shared_buffer)
{
    printf("+------------+------------+------------+\n");
    printf("| Commodity  | Price      | AVGPrice   |\n");
    printf("+------------+------------+------------+\n");
    for (int i = 0; i < shared_buffer->buffer_size; i++)
    {
        ProdCom commodity = shared_buffer->buffer[i];
        printf("| %-10s | %-10.2f | %-10.2f |\n", commodity.name, commodity.MeanPrice, commodity.AvgPrice);
    }
    printf("+------------+------------+------------+\n");
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <buffer_size>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int buffer_size = atoi(argv[1]);
    if (buffer_size <= 0)
    {
        fprintf(stderr, "Buffer size must be a positive integer.\n");
        exit(EXIT_FAILURE);
    }

    key_t key = ftok("producer.cpp", 65);
    if (key == -1)
        error_exit("ftok", NULL);

    int shmsize = sizeof(SharedBuffer) + buffer_size * sizeof(ProdCom);
    int shmid = shmget(key, shmsize, 0666);
    if (shmid == -1)
        error_exit("shmget", NULL);

    SharedBuffer *shared_buffer = (SharedBuffer *)shmat(shmid, NULL, 0);
    if (shared_buffer == (void *)-1)
        error_exit("shmat", shared_buffer);
    
    int semid = semget(key, 3, 0666);
    if (semid == -1)
        error_exit("semget", shared_buffer);
    
    

    while (true)
    {
        if (wait_sem(semid, SEM_FULL) == 0)
        {
            // Successfully decremented the semaphore
            // Safe to proceed with accessing the shared resource
            wait_sem(semid, SEM_MUTEX);

            // Critical section: consume item from the buffer
            ProdCom commodity = shared_buffer->buffer[shared_buffer->out];
            shared_buffer->out = (shared_buffer->out + 1) % shared_buffer->buffer_size;

            signal_sem(semid, SEM_MUTEX);
            signal_sem(semid, SEM_EMPTY);

            print_table(shared_buffer);
        }
        else
        {
            // Semaphore was unavailable, handle accordingly
            // You might log this event, try again later, or perform other tasks
            printf("Could not acquire semaphore, skipping this cycle.\n");
            // Optional: sleep for a short duration before trying again
            usleep(1000); // Sleep for 1 millisecond
        }
        sleep(1);
    }

    shmdt(shared_buffer);

    return 0;
}