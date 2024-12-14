#ifndef SHARED_H
#define SHARED_H

#include <semaphore.h> 
#include <sys/sem.h>
#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <time.h> 
#include <stdlib.h> 
#include <stdio.h>
#include <sys/mman.h> 
#include <string.h> 
#include <fcntl.h>  
#include <unistd.h> 
#include <stdarg.h> 
#include <iostream> 
#include <random>   
#include <errno.h>

#define MAX_HISTORY 5
#define MAX_COMMODITY_NAME_LENGTH 10
#define MAX_COMMODITIES 10
#define SEM_MUTEX 0 
#define SEM_FULL 2
#define SEM_EMPTY 1


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
    int in;
    int out;
    int buffer_size;
    ProdCom *buffer; 
    Semaphores sems;
} SharedBuffer;

typedef struct {
    char name[20];    
    float prices[MAX_HISTORY];  
    int index;        
} CommodityHistory;

void error_exit(const char *msg, SharedBuffer *shared_buffer)
{
    shmdt(shared_buffer);
    perror(msg);
    exit(EXIT_FAILURE);
}
int wait_sem(int semid, int semnum)
{
    struct sembuf op = {semnum, -1, IPC_NOWAIT};
    if (semop(semid, &op, 1) == -1)
    {
        if (errno == EAGAIN)
        {
            return -1; // operation blocked
        }
        else
        {
            error_exit("semop", NULL);
        }
    }
    return 0;
}

int signal_sem(int semid, int semnum)
{
    struct sembuf op = {semnum, 1, IPC_NOWAIT};
    if (semop(semid, &op, 1) == -1)
    {
        if (errno == EAGAIN)
        {
            return -1; // operation blocked
        }
        else
            error_exit("semop", NULL);
    }
    return 0;
}


#endif
