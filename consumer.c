#include "shared.h"
/*consumer takes in as an arguent the size of the buffer only and prints*/
/* outputs a table in the form
+------------+------------+------------+
| Commodity  | Price      | AVGPrice   |
+------------+------------+------------+
| Aluminum   | 1000.00 ↓  | 1000.00 ↓  |
| Silver     | 500.00 ↑   | 500.00 ↑   |
| Copper     | 0.0        | 0.0        |
| Gold       | 0.0        | 0.0        |
| CrudeOil   | 0.0        | 0.0        |
| NaturalGas | 0.0        | 0.0        |
| Nickel     | 0.0        | 0.0        |
| Zinc       | 0.0        | 0.0        |
| Lead       | 0.0        | 0.0        |
| Methaoil   | 0.0        | 0.0        |
| Cotton     | 0.0        | 0.0        |
+------------+------------+------------+

*/
// build with gcc -o consumer consumer.c -Wall -Wextra -std=c99
// run with ./consumer <buffer_size>
// example: ./consumer 40
int main(int argc, char *argv[]){
    if(argc != 2){
        fprintf(stderr, "Usage: %s <buffer_size>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int buffer_size = atoi(argv[1]);
    key_t key = ftok("producer.c", 65);
    if(key == -1){
        error_exit("ftok");
    }
    size_t shmsize = sizeof(SharedBuffer) + buffer_size * sizeof(Commodity);
    int shmid = shmget(key, shmsize, 0666);
    if(shmid == -1){
        error_exit("shmget");
    }
    SharedBuffer *shared_buffer = (SharedBuffer *)shmat(shmid, NULL, 0);
    if(shared_buffer == (void *)-1){
        error_exit("shmat");
    }
    int semid = semget(key, 3, 0666);
    if(semid == -1){
        error_exit("semget");
    }
    printf("+------------+------------+------------+\n");
    printf("| Commodity  | Price      | AVGPrice   |\n");
    printf("+------------+------------+------------+\n");
    for(int i = 0; i < MAX_COMMODITIES; i++){
        wait_sem(semid, SEM_FULL);
        wait_sem(semid, SEM_MUTEX);
        Commodity *commodity = &shared_buffer->buffer[shared_buffer->out];
        shared_buffer->out = (shared_buffer->out + 1) % shared_buffer->buffer_size;
        signal_sem(semid, SEM_MUTEX);
        signal_sem(semid, SEM_EMPTY);
        printf("| %-10s | %8.2f   | %8.2f   |\n", commodity->name, commodity->price, commodity->price);
    }
    printf("+------------+------------+------------+\n");
    return 0;   
}