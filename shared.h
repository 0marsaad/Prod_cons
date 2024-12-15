/*
    This file contains the shared data structures and functions used by the producer and consumer processes.
    The shared data structures are:
    1. ProdCom: A structure that represents a commodity. It contains the name of the commodity, the average price, the mean price, the standard deviation, the sleep interval, and the timestamp.
    2. Semaphores: A structure that contains the semaphores used for synchronization.
    3. SharedBuffer: A structure that contains the shared buffer used for communication between the producer and consumer processes.
    4. CommodityHistory: A structure that contains the history of a commodity. It contains the name of the commodity, the prices, and the index.
    The functions are:
    1. error_exit: A function that prints an error message and exits the program.
    2. wait_sem: A function that waits on a semaphore.
    3. signal_sem: A function that signals a semaphore.
*/

#ifndef SHARED_H
#define SHARED_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <random>
#include <errno.h>

#define MAX_COMMODITY_NAME_LENGTH 11
#define MAX_COMMODITIES 11

// El semaphors 3 
#define SEM_MUTEX 0
#define SEM_EMPTY 1
#define SEM_FULL 2

// Commodity struct
typedef struct
{
    char name[MAX_COMMODITY_NAME_LENGTH];
    double Price;
} ProdCom;

// hena 3mlt array of commodities
typedef struct
{
    ProdCom buffer[1]; // el 1 place holder haye7sal realloc fel initialization

} SharedBuffer;

//dol el global variables
extern int shmid;
extern int semid;
extern SharedBuffer *shared_buffer;

union semun{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};


// Error handling
void error_exit(const char *msg, SharedBuffer *shared_buffer);

// bet initialize el semaphores
void init_sem(int empty, int full);

// Initialize shared memory
void init_shm(int buffer_size);

// Wait on semaphore
void wait_sem(int semnum);

// Signal semaphore
void signal_sem(int semnum);

#endif

