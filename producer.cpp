/*
    this is the producer process
    it generates random prices for commodities and writes them to the shared buffer
    it also logs the actions it takes
    it takes in as an argument
    1. the name of a commodity it is responsible of updating
    2. the mean price of the commodity
    3. the standard deviation of the commodity
    4. the sleep interval
    5. the size of the buffer
*/


#include "shared.h" // for SharedBuffer, ProdCom, Semaphores


void log_action(const char *action, const char *commodity, double price, const char *format, ...)
{
    struct timespec timeStamp;
    clock_gettime(CLOCK_REALTIME, &timeStamp);

    char time_buffer[64];
    struct tm *tm_info = localtime(&timeStamp.tv_sec);
    strftime(time_buffer, sizeof(time_buffer), "[%m/%d/%Y %H:%M:%S", tm_info);

    // Get milliseconds
    long milliseconds = timeStamp.tv_nsec / 1000000;

    // Prepare the variable argument list
    va_list args;
    va_start(args, format);

    // Print to stderr
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

    srand(time(NULL) ^ getpid());

    if (SleepInterval < 0 || buffer_size <= 0 || MeanPrice <= 0 || StdDev <= 0 || StdDev > MeanPrice)
    {
        fprintf(stderr, "Invalid arguments\n");
        exit(EXIT_FAILURE);
    }

    key_t key = ftok("producer.cpp", 65);
    printf("key: %d\n", key);
    if (key == -1)
        error_exit("ftok", NULL);

    size_t shmsize = sizeof(SharedBuffer) + buffer_size * sizeof(ProdCom);
    printf("shmsize: %d\n", shmsize);
    int shmid = shmget(key, shmsize, IPC_CREAT | IPC_EXCL | 0666);
    bool exists = false;
    printf("shmid: %d\n", shmid);
    if (shmid == -1)
    {
        if (errno == EEXIST)
        {
            shmid = shmget(key, shmsize, 0666);
            printf("shmid: %d\n", shmid);

            if (shmid == -1)
                error_exit("shmget", NULL);
        }
        else
            error_exit("shmget", NULL);
    }
    else
    {
        exists = true;
    }

    SharedBuffer *shared_buffer = (SharedBuffer *)shmat(shmid, NULL, 0);
    if (shared_buffer == (void *)-1)
        error_exit("shmat", shared_buffer);
    if (exists)
    {
        printf("exists\n");
        if (shared_buffer->buffer_size != buffer_size)
        {
            shmdt(shared_buffer);
            shmctl(shmid, IPC_RMID, NULL);
            shmid = shmget(key, shmsize, IPC_CREAT | IPC_EXCL | 0666);
            if (shmid == -1)
                error_exit("shmget", shared_buffer);
            shared_buffer = (SharedBuffer *)shmat(shmid, NULL, 0);
            if (shared_buffer == (void *)-1)
                error_exit("shmat", shared_buffer);
        }
    }
    else
    {
        shared_buffer->buffer_size = buffer_size;
        shared_buffer->in = 0;
        shared_buffer->out = 0;
        shared_buffer->buffer = (ProdCom *)(shared_buffer + 1);
    }

    int semid = semget(key, 3, 0666 | IPC_CREAT);
    printf("semid: %d\n", semid);

    if (semid == -1)
    {
        if (errno == EEXIST)
        {
            semid = semget(key, 3, 0666);
            if (semid == -1)
                error_exit("semget", shared_buffer);
        }
        else
        {
            error_exit("semget", shared_buffer);
        }
    }
    else
    {
    }

    if (sem_init(&shared_buffer->sems.mutex, 1, 1) == -1)
        error_exit("sem_init", shared_buffer);
    if (sem_init(&shared_buffer->sems.empty, 1, buffer_size) == -1)
        error_exit("sem_init", shared_buffer);
    if (sem_init(&shared_buffer->sems.full, 1, 0) == -1)
        error_exit("sem_init", shared_buffer);

    ProdCom commodity;
    strncpy(commodity.name, name, MAX_COMMODITY_NAME_LENGTH);
    commodity.MeanPrice = MeanPrice;
    commodity.StdDev = StdDev;
    commodity.SleepInterval = SleepInterval;

    int CircQueue[4] = {0};
    double avg_price = 0;
    int count = 0;

    while (true)
    {
        log_action("Producer", name, commodity.MeanPrice, "Generated price: %.2f", commodity.MeanPrice);
        if (wait_sem(semid, SEM_EMPTY) == 0)
        {
            // Successfully decremented the semaphore
            // Safe to proceed with accessing the shared resource
            wait_sem(semid, SEM_MUTEX);

            // Critical section: produce item and place it in the buffer
            shared_buffer->buffer[shared_buffer->in] = commodity;
            shared_buffer->in = (shared_buffer->in + 1) % shared_buffer->buffer_size;

            if(signal_sem(semid, SEM_MUTEX) == -1)
            {
                // Semaphore was unavailable, handle accordingly
                // You might log this event, try again later, or perform other tasks
                printf("Could not release semaphore, skipping this cycle.\n");
                // Optional: sleep for a short duration before trying again
                usleep(1000); // Sleep for 1 millisecond
            }
        }
        else
        {
            // Semaphore was unavailable, handle accordingly
            // You might log this event, try again later, or perform other tasks
            printf("Could not acquire semaphore, skipping this cycle.\n");
            // Optional: sleep for a short duration before trying again
            usleep(1000); // Sleep for 1 millisecond
        }
        log_action("Producer", name, commodity.MeanPrice, "Writing to buffer");
        wait_sem(semid, SEM_MUTEX);

        if (count == 4)
            count = 0;
        printf("count: %d\n", count);
        CircQueue[count] = generate_price(MeanPrice, StdDev);
        avg_price = (CircQueue[0] + CircQueue[1] + CircQueue[2] + CircQueue[3]) / 4;
        commodity.AvgPrice = avg_price;
        log_action("Producer", name, commodity.MeanPrice, "Average price: %.2f", avg_price);
        commodity.timestamp = time(NULL);
        commodity.MeanPrice = CircQueue[count];

        shared_buffer->buffer[shared_buffer->in] = commodity;
        shared_buffer->in = (shared_buffer->in + 1) % shared_buffer->buffer_size;

        signal_sem(semid, SEM_MUTEX);
        signal_sem(semid, SEM_FULL);

        count++;
        sleep(SleepInterval);
    }
    shmdt(shared_buffer);
    return 0;
}