#define _POSIX_C_SOURCE 199309L

#include "shared.h"
#include <time.h>
#include <stdarg.h>
#include <math.h>
#include <stdlib.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void log_message(const char *commodity, const char *format, ...) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    char time_buffer[64];
    struct tm *tm_info = localtime(&ts.tv_sec);
    strftime(time_buffer, sizeof(time_buffer), "[%m/%d/%Y %H:%M:%S", tm_info);

    // Get milliseconds
    long milliseconds = ts.tv_nsec / 1000000;

    // Prepare the variable argument list
    va_list args;
    va_start(args, format);

    // Print to stderr
    fprintf(stderr, "%s.%03ld] %s: ", time_buffer, milliseconds, commodity);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");

    va_end(args);
}

double generate_normal(double mean, double stddev) {
    static int have_spare = 0;
    static double rand1, rand2;

    if (have_spare) {
        have_spare = 0;
        return mean + stddev * sqrt(rand1) * sin(rand2);
    }

    have_spare = 1;

    rand1 = rand() / ((double)RAND_MAX);
    if (rand1 < 1e-100) rand1 = 1e-100;
    rand1 = -2 * log(rand1);
    rand2 = (rand() / ((double)RAND_MAX)) * M_PI * 2;

    return mean + stddev * sqrt(rand1) * cos(rand2);
}

/*producer.c takes in the name of the commodity
 and the average price of the commodity and the standard diviation of the price commodity 
 and the sleep interval of the commodity and the bound buffer size for the shared memory*/
// run by ./producer <commodity> <average_price> <std_dev> <sleep_interval> <buffer_size>
// example: ./producer gold 1000 100 1 10
// build by gcc -Wall -g -o producer producer.c -lrt -lm -lpthread
int main(int argc, char *argv[]) {
    if (argc != 6) {
        fprintf(stderr, "Usage: %s <commodity_name> <mean_price> <std_dev> <sleep_interval_ms> <buffer_size>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
//     get the arguments
    char *commodity_name = argv[1];
    double mean_price = atof(argv[2]);
    double std_dev = atof(argv[3]);
    int sleep_interval = atoi(argv[4]);
    int buffer_size = atoi(argv[5]);
//     seed the random number generator
    srand(time(NULL) ^ getpid());
//     get the key
    key_t key = ftok("producer.c", 65);
    if (key == -1) {
        error_exit("ftok");
    }
//     get the shared memory
    size_t shmsize = sizeof(SharedBuffer) + buffer_size * sizeof(Commodity);
    if (shmsize < sizeof(SharedBuffer)) {
        error_exit("buffer size too large");
    }
    
    int shmid = shmget(key, shmsize, IPC_CREAT | 0666);
    if (shmid == -1) {
        error_exit("shmget");
    }
//     attach the shared memory
    SharedBuffer *shared_buffer = (SharedBuffer *)shmat(shmid, NULL, 0);
    if (shared_buffer == (void *)-1) {
        error_exit("shmat");
    }
//     initialize the shared buffer
    static int initialized = 0;
    if (!initialized) {
        shared_buffer->in = 0;
        shared_buffer->out = 0;
        shared_buffer->buffer_size = buffer_size;
        initialized = 1;
    }
//     get the semaphore
    int semid = semget(key, 3, IPC_CREAT | 0666);
    if (semid == -1) {
        error_exit("semget");
    }
//     set the semaphore values
    while (1) {
        double price = generate_normal(mean_price, std_dev);
        log_message(commodity_name, "generating a new value %.2f", price);

        log_message(commodity_name, "trying to get mutex on shared buffer");
        wait_sem(semid, SEM_EMPTY);

        wait_sem(semid, SEM_MUTEX);

        shared_buffer->buffer[shared_buffer->in].price = price;
        shared_buffer->buffer[shared_buffer->in].timestamp = time(NULL);
        strncpy(shared_buffer->buffer[shared_buffer->in].name, commodity_name, MAX_COMMODITY_NAME_LENGTH);
        shared_buffer->in = (shared_buffer->in + 1) % shared_buffer->buffer_size;

        log_message(commodity_name, "placing %.2f on shared buffer", price);

        signal_sem(semid, SEM_MUTEX);
        signal_sem(semid, SEM_FULL);

        log_message(commodity_name, "sleeping for %d ms", sleep_interval);
        sleep(sleep_interval * 1000);
    }

    return 0;
}



