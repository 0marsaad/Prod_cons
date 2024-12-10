#ifndef SHARED_H
#define SHARED_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>    // For sleep function
#include <time.h>      // For time functions
#include <sys/ipc.h>   // For IPC mechanisms
#include <sys/shm.h>   // For shared memory
#include <sys/sem.h>   // For semaphores
#include <sys/types.h>
#include <errno.h>
#include <stdarg.h>

/* Constants */
#define MAX_COMMODITY_NAME_LENGTH 11  // 10 characters + null terminator
#define MAX_COMMODITIES 20            // Number of commodities

/* Semaphore indices */
#define SEM_MUTEX 0
#define SEM_EMPTY 1
#define SEM_FULL  2

/* Union for semaphore operations */
union semun {
    int val;                // Value for SETVAL
    struct semid_ds *buf;   // Buffer for IPC_STAT, IPC_SET
    unsigned short *array;  // Array for GETALL, SETALL
};

/* Commodity price structure */
typedef struct {
    char name[MAX_COMMODITY_NAME_LENGTH];
    double price;
    time_t timestamp;
} Commodity;

/* Shared buffer structure with a flexible array member */
typedef struct {
    int in;
    int out;
    int buffer_size;
    Commodity buffer[];  // Flexible array member
} SharedBuffer;

/* Function to handle errors */
void error_exit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void wait_sem(int semid, int semnum) {
    struct sembuf sem_op;
    sem_op.sem_num = semnum;
    sem_op.sem_op = -1;
    sem_op.sem_flg = 0;
    if (semop(semid, &sem_op, 1) == -1) {
        error_exit("semop");
    }
}
void signal_sem(int semid, int semnum) {
    struct sembuf sem_op;
    sem_op.sem_num = semnum;
    sem_op.sem_op = 1;
    sem_op.sem_flg = 0;
    if (semop(semid, &sem_op, 1) == -1) {
        error_exit("semop");
    }
}

#endif  // SHARED_H