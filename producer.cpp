/*
    Producer program for the commodity price monitoring system.
    This program generates random prices for a given commodity and stores them in a shared buffer.
    The shared buffer is read by the consumer program.

    Usage: ./producer <commodity_name> <mean_price> <std_dev> <sleep_interval> <buffer_size>

    build: g++ -o producer producer.cpp -lrt
*/

#include "shared.h"

#include <stdarg.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <iostream>
#include <random>

int semid = -1;
int shmid = -1;
SharedBuffer *shared_buffer = NULL;

void log_action(const char *action, const char *commodity, double price, const char *format, ...)
{
    struct timespec timeStamp;
    clock_gettime(CLOCK_REALTIME, &timeStamp);

    char time_buffer[64];
    struct tm *tm_info = localtime(&timeStamp.tv_sec);
    strftime(time_buffer, sizeof(time_buffer), "[%m/%d/%Y %H:%M:%S", tm_info);

    long milliseconds = timeStamp.tv_nsec / 1000000;

    va_list args;
    va_start(args, format);

    fprintf(stderr, "%s.%03ld] %s: ", time_buffer, milliseconds, commodity);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");

    va_end(args);
}

double generate_price(double mean, double stddev)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::normal_distribution<> dist(mean, stddev);
    return dist(gen);
}

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
        error_exit("semctl-set-mutex prod", shared_buffer);
    arg.val = empty;
    if (semctl(semid, SEM_EMPTY, SETVAL, arg) == -1)
        error_exit("semctl-set-empty prod", shared_buffer);
    arg.val = full;
    if (semctl(semid, SEM_FULL, SETVAL, arg) == -1)
        error_exit("semctl-set-full prod", shared_buffer);
}

void init_shm (int buffer_size){
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

int main(int argc, char *argv[])
{
    if (argc != 6)
    {
        fprintf(stderr, "Usage: %s <commodity_name> <mean_price> <std_dev> <sleep_interval> <buffer_size>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char name[MAX_COMMODITY_NAME_LENGTH];
    strncpy(name, argv[1], MAX_COMMODITY_NAME_LENGTH);
    double MeanPrice = atof(argv[2]);
    double StdDev = atof(argv[3]);
    int SleepInterval = atoi(argv[4]);
    int buffer_size = atoi(argv[5]);

    // hena hainitialize el semaphores
    init_sem(buffer_size, 0);
    init_shm(buffer_size);

    while(true){
        double newPrice = generate_price(MeanPrice, StdDev);
        
        struct timespec timeStamp;
        clock_gettime(CLOCK_REALTIME, &timeStamp);
        char time_buffer[64];
        strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", localtime(&timeStamp.tv_sec));

        log_action("Producer", name, newPrice, "Generated price: %.2f", newPrice);

        wait_sem(SEM_EMPTY);
        
        log_action("Producer", name, newPrice, "Acquired SEM_EMPTY getting SEM_MUTEX");
        wait_sem(SEM_MUTEX);

        log_action("Producer", name, newPrice, "Acquired SEM_MUTEX");

        shared_buffer->buffer[0].Price = newPrice;
        strncpy(shared_buffer->buffer[0].name, name, MAX_COMMODITY_NAME_LENGTH);
        
        log_action("Producer", name, newPrice, "Added to buffer");

        signal_sem(SEM_MUTEX);
        log_action("Producer", name, newPrice, "Released SEM_MUTEX");
        signal_sem(SEM_FULL);
        log_action("Producer", name, newPrice, "Released SEM_FULL");

        usleep(SleepInterval);  
    }
    return 0;
}
