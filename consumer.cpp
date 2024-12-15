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

// build with g++ o consumer consumer.c -lrt
// run with ./consumer <buffer_size>
// example: ./consumer 40

#include "shared.h"

const char *commodities[MAX_COMMODITIES] = {
    "ALUMINUM",
    "SILVER",
    "COPPER",
    "GOLD",
    "CRUDEOIL",
    "NATURALGAS",
    "NICKEL",
    "ZINC",
    "LEAD",
    "METHANOL",
    "COTTON"};

int semid = -1;
int shmid = -1;
SharedBuffer *shared_buffer = NULL;

void error_exit(const char *msg, SharedBuffer *shared_buffer)
{
    if (shared_buffer != NULL)
        shmdt(shared_buffer);
    if (shmid != -1)
        shmctl(shmid, IPC_RMID, NULL);
    if (semid != -1)
        semctl(semid, 0, IPC_RMID);
    perror(msg);
    exit(EXIT_FAILURE);
}

void init_sem(int empty, int full)
{
    key_t key = ftok("Elgoat", 65);
    semid = semget(key, 3, 0666 | IPC_CREAT);
    if (semid == -1)
        error_exit("semget", shared_buffer);

    union semun arg;
    arg.val = 1;
    if (semctl(semid, SEM_MUTEX, SETVAL, arg) == -1)
        error_exit("semctl-set-mutex", shared_buffer);
    arg.val = empty;
    if (semctl(semid, SEM_EMPTY, SETVAL, arg) == -1)
        error_exit("semctl-set-empty", shared_buffer);
    arg.val = full;
    if (semctl(semid, SEM_FULL, SETVAL, arg) == -1)
        error_exit("semctl-set-full", shared_buffer);
}

void init_shm(int buffer_size)
{
    key_t key = ftok("Elgoat2", 75);
    size_t shmsize = sizeof(SharedBuffer) + buffer_size * sizeof(ProdCom);

    shmid = shmget(key, shmsize, IPC_CREAT | 0666);

    if (shmid == -1)
        error_exit("shmget", NULL);

    shared_buffer = (SharedBuffer *)shmat(shmid, NULL, 0);

    if (shared_buffer == (void *)-1)
        error_exit("shmat", shared_buffer);

    memset(shared_buffer, 0, shmsize);
}

void wait_sem(int semnum)
{
    struct sembuf op = {semnum, -1, 0};
    semop(semid, &op, 1);
}

void signal_sem(int semnum)
{
    struct sembuf op = {semnum, 1, 0};
    semop(semid, &op, 1);
}

static double akherPrice[MAX_COMMODITIES] = {0.0};
static double taree5Price[MAX_COMMODITIES][5] = {{0.0}};
static int indexP[MAX_COMMODITIES] = {0};



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
    init_sem(buffer_size, 0);
    init_shm(buffer_size);


    
    while (true)
    {
        wait_sem(SEM_FULL);
        wait_sem(SEM_MUTEX);

        char name[MAX_COMMODITY_NAME_LENGTH];
        double price = shared_buffer->buffer[0].Price;
        strncpy(name, shared_buffer->buffer[0].name, MAX_COMMODITY_NAME_LENGTH);

        signal_sem(SEM_MUTEX);
        signal_sem(SEM_EMPTY);

        for (int i = 0; i < MAX_COMMODITIES; i++){
            if (strcmp(name, commodities[i]) == 0){
                int ind = indexP[i] % 5;
                taree5Price[i][ind] = price;
                indexP[i]++;
                break;
            }
        }
        printf("\e[1;1H\e[2J");
        printf("+------------+--------------+--------------+\n");
        printf("| Commodity  | Price        | AVGPrice     |\n");
        printf("+------------+--------------+--------------+\n");

        for (int i = 0; i < MAX_COMMODITIES; i++)
        {
            double price7aly = 0.0;
            double avgPrice = 0.0;
            int indcnt = (indexP[i] > 5) ? 5 : indexP[i];

            if(indcnt > 0){
                int current = indexP[i] % 5;
                price7aly = taree5Price[i][current];
            }

            double sum = 0.0;
            for (int j = 0; j < indcnt; j++)
            {
                sum += taree5Price[i][j];
            }
            avgPrice = sum / indcnt;

            char* arrow7aly = " ";
            char* arrowavg = "";
            if (price7aly > akherPrice[i])
                arrow7aly = "↑";
            else if (price7aly < akherPrice[i])
                arrow7aly = "↓";

            if (avgPrice > akherPrice[i])
                arrowavg = "↑";
            else if (avgPrice < akherPrice[i])
                arrowavg = "↓";

            akherPrice[i] = price7aly;

            printf("| %-10s | %10.2f %s | %10.2f %s |\n", commodities[i], price7aly, arrow7aly, avgPrice, arrowavg);

           
            

        }
        printf("+------------+--------------+--------------+\n");

        usleep(200000);
    }

            return 0;
}