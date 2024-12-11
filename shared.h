#ifndef SHARED_H
#define SHARED_H

#include <semaphore.h> // for sem_t
#include <sys/sem.h> // for sembuf
#include <sys/types.h> // for key_t
#include <sys/ipc.h> // for ftok
#include <sys/shm.h> // for shmget, shmat
#include <time.h> // for time
#include <stdlib.h> // for exit
#include <stdio.h> // for fprintf
#include <sys/mman.h> // for mmap
#include <string.h> // for memset
#include <unistd.h> // for sleep
#include <fcntl.h>  // for O_CREAT

#define MAX_COMMODITY_NAME_LENGTH 10
#define MAX_COMMODITIES 10
#define SEM_MUTEX 0
#define SEM_EMPTY 1
#define SEM_FULL 2


typedef struct {
    char name[MAX_COMMODITY_NAME_LENGTH];
    double AvgPrice;
    double MeanPrice;
    double StdDev;
    int SleepInterval;
    time_t timestamp;
} ProdCom;


typedef struct {
    sem_t mutex;
    sem_t empty;
    sem_t full;
} Semaphores;

typedef struct{
    ProdCom *buffer;
    int in;
    int out;
    int buffer_size;
    Semaphores sems;
} SharedBuffer;

#endif