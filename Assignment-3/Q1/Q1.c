#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_PHILOSOPHERS 5
#define NUM_BOWLS 2

pthread_mutex_t forks[NUM_PHILOSOPHERS];
pthread_mutex_t bowls[NUM_BOWLS];
pthread_cond_t bowl_available[NUM_BOWLS];

void thinking(int philosopher_id) {
    // Simulate thinking
    printf("Philosopher %d is thinking...\n",philosopher_id);
    usleep(rand() % 1000000); // Sleep for a random time
}

void eating(int philosopher_id) {
    // Simulate eating
    printf("Philosopher %d is eating...\n", philosopher_id);
    usleep(rand() % 1000000); // Sleep for a random time
}

void* philosopher(void* args) {
    int philosopher_id = *((int*)args);
    int left_fork = philosopher_id;
    int right_fork = (philosopher_id + 1) % NUM_PHILOSOPHERS;
    int bowl_id = philosopher_id % NUM_BOWLS;

    for(int i=0;i<50;i++) {
        thinking(philosopher_id);

        // Pick up forks
        pthread_mutex_lock(&forks[left_fork]);
        pthread_mutex_lock(&forks[right_fork]);

        // Check if the bowl is available
        while (pthread_mutex_trylock(&bowls[bowl_id]) != 0) {
            // If the bowl is not available, release the forks and wait for a signal
            pthread_mutex_unlock(&forks[left_fork]);
            pthread_mutex_unlock(&forks[right_fork]);
            printf("Philosopher %d is waiting for a bowl...\n", philosopher_id);
            pthread_cond_wait(&bowl_available[bowl_id], &forks[left_fork]);
        }

        // Start eating
        eating(philosopher_id);

        // Put down forks and release the bowl
        pthread_mutex_unlock(&forks[left_fork]);
        pthread_mutex_unlock(&forks[right_fork]);
        pthread_mutex_unlock(&bowls[bowl_id]);

        // Signal that the bowl is available for other philosophers
        pthread_cond_signal(&bowl_available[bowl_id]);
    }

    return NULL;
}

int main() {
    pthread_t philosophers[NUM_PHILOSOPHERS];
    int philosopher_ids[NUM_PHILOSOPHERS];

    // Initialize forks and bowls
    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        pthread_mutex_init(&forks[i], NULL);
    }

    for (int i = 0; i < NUM_BOWLS; ++i) {
        pthread_mutex_init(&bowls[i], NULL);
        pthread_cond_init(&bowl_available[i], NULL);
    }

    // Create philosopher threads
    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        philosopher_ids[i] = i;
        pthread_create(&philosophers[i], NULL, philosopher, &philosopher_ids[i]);
    }

    // Allow philosophers to attempt to eat initially
    sleep(2);

    // Signal that the bowls are initially available
    for (int i = 0; i < NUM_BOWLS; ++i) {
        pthread_cond_signal(&bowl_available[i]);
    }

    // Wait for threads to finish (This won't happen in this example as philosophers run forever)
    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        pthread_join(philosophers[i], NULL);
    }

    // Cleanup
    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        pthread_mutex_destroy(&forks[i]);
    }

    for (int i = 0; i < NUM_BOWLS; ++i) {
        pthread_mutex_destroy(&bowls[i]);
        pthread_cond_destroy(&bowl_available[i]);
    }

    return 0;
}
