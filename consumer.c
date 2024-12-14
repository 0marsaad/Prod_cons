#include "shared.h"
  
CommodityHistory commHis[10] = {
    {"ALUMINIUM", {0.0}, 0},
    {"CRUDEOIL", {0.0}, 0},
    {"COPPER", {0.0}, 0},
    {"GOLD", {0.0}, 0},
    {"LEAD", {0.0}, 0},
    {"MENTHAOIL", {0.0}, 0},
    {"NICKEL", {0.0}, 0},
    {"NATURALGAS", {0.0}, 0},
    {"SILVER", {0.0}, 0},
    {"ZINC", {0.0}, 0}
};  

void print_table(SharedBuffer *shared_buffer) {
    printf("\e[1;1H\e[2J"); // Clear el terminal (given)
    printf("+-----------------+-----------+-----------+\n");
    printf("| Commodity       | Price     | AvgPrice  |\n");
    printf("+-----------------+-----------+-----------+\n");

    for (int i = 0; i < 10; i++) {
        ProdCom *commodity = NULL;
        for (int j = 0; j < shared_buffer->buffer_size; j++) {
            if (strcmp(shared_buffer->buffer[j].name, commHis[i].name) == 0) {
                commodity = &shared_buffer->buffer[j];
                break;
            }
        }

        if (commodity != NULL) {
            // Update prices ll commodity
            commHis[i].prices[commHis[i].index] = commodity->AvgPrice;
            commHis[i].index = (commHis[i].index + 1) % MAX_HISTORY;

            // Calc el avg price from last 5 readings
            float sumPrice = 0.0;
            int count = 0;
            for (int j = 0; j < MAX_HISTORY; j++) {
                if (commHis[i].prices[j] != 0.0) {
                    sumPrice += commHis[i].prices[j];
                    count++;
                }
            }
            char arrStat = ' ';  // No change ll arrow
            float avgPrice = (count > 0) ? sumPrice / count : 0.0;
            if (count > 1) {
                float lastPrice = commHis[i].prices[(commHis[i].index - 2 + MAX_HISTORY) % MAX_HISTORY];
                if (commodity->AvgPrice > lastPrice) {
                    arrStat = '↑';  
                } else if (commodity->AvgPrice < lastPrice) {
                    arrStat = '↓';  
                }
            }

            printf("| %-15s | %-9.2f | %-9.2f%c |\n", commodity->name, commodity->AvgPrice, avgPrice, arrStat);
        } else {
            printf("| %-15s | %-9.2f | %-9.2f |\n", commHis[i].name, 0.0, 0.0);
        }
    }

    printf("+-----------------+-----------+-----------+\n");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <buffer_size>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int buffer_size = atoi(argv[1]);
    if (buffer_size <= 0) {
        fprintf(stderr, "Buffer size must be a positive integer.\n");
        exit(EXIT_FAILURE);
    }

    // b Obtain el shared memory key(same zy el producer)
    key_t key = ftok("producer.cpp", 65);
    if (key == -1) error_exit("ftok", NULL);

    // ba cal el size needed for SharedBuffer + buffer_size ProdCom entries
    int shmsize = sizeof(SharedBuffer) + buffer_size * sizeof(ProdCom);

    // Access shared memory
    int shmid = shmget(key, shmsize, 0666);
    if (shmid == -1) error_exit("shmget", NULL);

    // Attach to shared memory
    SharedBuffer *shared_buffer = (SharedBuffer *)shmat(shmid, NULL, 0);
    if (shared_buffer == (void *)-1) {
      perror("shmat failed");
      exit(EXIT_FAILURE);
    }

    // b3dl el buffer pointer ll makn el ba3do
    shared_buffer->buffer = (ProdCom *)((char *)shared_buffer + sizeof(SharedBuffer));

    // Access the semaphore
    int semid = semget(key, 3, 0666);
    if (semid == -1) error_exit("semget", shared_buffer);

    // Main consumer loop
    while (true) {
        if (wait_sem(semid, SEM_FULL) == 0) {
            wait_sem(semid, SEM_MUTEX); // bt Lock el buffer access

            // Consume item
            printf("Semaphore acquired, entering critical section...\n");
            print_table(shared_buffer);

            // inc. the out index forward
            shared_buffer->out = (shared_buffer->out + 1) % shared_buffer->buffer_size;
            signal_sem(semid, SEM_MUTEX); 
            signal_sem(semid, SEM_EMPTY);

            sleep(1);
        } else {
            usleep(1000); // Sleep
        }
    }

    // b clear buffer
    shmdt(shared_buffer);

    return 0;
}

