#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

static void *
worker(void *arg)
{
    double pi = 0.0;
    double denom = 1.0;

    for (unsigned i = 0; i < 100000000; i++) {
        pi += 4.0 / denom;
        pi -= 4.0 / (denom + 2.0);
        denom += 4.0;
    }

    printf("pi=%f\n", pi);
    return NULL;
}

static void *
high_priority(void *arg)
{
    unsigned long last = clock_gettime_mon_ns();
    for (;;) {
        sleep(1);
        unsigned long now = clock_gettime_mon_ns();
        printf("Slept for %luus\n", (now - last) / 1000UL);
        last = now;
    }

    return NULL;
}

int
main(int argc, char **argv)
{
    // Attribute structure for a high-priority thread.
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

    struct sched_param sched = { .sched_priority = 63 };
    pthread_attr_setschedparam(&attr, &sched);

    // Create the high-priority thread.
    pthread_t tids[11];
    int rc;
    rc = pthread_create(&tids[0], &attr, high_priority, NULL);
    if (rc != 0) {
        fprintf(stderr, "pthread_create: %s\n", strerror(rc));
        return EXIT_FAILURE;
    }

    // Create worker threads.
    for (unsigned i = 1; i < 11; i++) {
        rc = pthread_create(&tids[i], NULL, worker, NULL);
        if (rc != 0) {
            fprintf(stderr, "pthread_create: %s\n", strerror(rc));
            return EXIT_FAILURE;
        }
    }

    // Wait for workers to finish.
    for (unsigned i = 1; i < 11; i++) {
        pthread_join(tids[i], NULL);
    }

    return EXIT_SUCCESS;
}
