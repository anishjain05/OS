#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_CARS 5

sem_t bridge;
sem_t mutex;
int on_bridge = 0; // Number of cars currently on the bridge
int left_cars, right_cars;
int total_crossed = 0;

void crossing(int direction, int car_id) {
    printf("Car %d is crossing from %s to %s\n", car_id,
           direction == 1 ? "left" : "right", direction == 1 ? "right" : "left");
    usleep(rand() % 1000000); // Simulate crossing time
}

void* left(void* args) {
    int car_id = *((int*)args);
    while (1) {
        sem_wait(&mutex);
        if (on_bridge < MAX_CARS && left_cars > 0) {
            left_cars--;
            on_bridge++;
            sem_post(&mutex);

            sem_wait(&bridge);
            crossing(1, car_id);
            sem_post(&bridge);

            sem_wait(&mutex);
            on_bridge--;
            total_crossed++;

            // Check if all cars have crossed
            if (total_crossed == left_cars + right_cars) {
                sem_post(&mutex);
                pthread_exit(NULL);
                exit(1);
            }
            sem_post(&mutex);

            usleep(rand() % 1000000); // Simulate time before the next car arrives
        } else {
            sem_post(&mutex);
            usleep(100000); // Sleep for a short time if the bridge is full or no left cars
        }
    }
}

void* right(void* args) {
    int car_id = *((int*)args);
    while (1) {
        sem_wait(&mutex);
        if (on_bridge < MAX_CARS && right_cars > 0) {
            right_cars--;
            on_bridge++;
            sem_post(&mutex);

            sem_wait(&bridge);
            crossing(2, car_id);
            sem_post(&bridge);

            sem_wait(&mutex);
            on_bridge--;
            total_crossed++;

            // Check if all cars have crossed
            if (total_crossed == left_cars + right_cars) {
                sem_post(&mutex);
                pthread_exit(NULL);
                exit(1);
            }
            sem_post(&mutex);

            usleep(rand() % 1000000); // Simulate time before the next car arrives
        } else {
            sem_post(&mutex);
            usleep(100000); // Sleep for a short time if the bridge is full or no right cars
        }
    }
}

int main() {
    srand(time(NULL)); // Seed the random number generator

    sem_init(&bridge, 0, MAX_CARS);
    sem_init(&mutex, 0, 1);

    // Get the number of left and right cars from the user
    printf("Enter the number of cars on the left: ");
    scanf("%d", &left_cars);

    printf("Enter the number of cars on the right: ");
    scanf("%d", &right_cars);

    pthread_t left_threads[left_cars], right_threads[right_cars];
    int left_ids[left_cars], right_ids[right_cars];

    for (int i = 0; i < left_cars; i++) {
        left_ids[i] = i + 1;
        pthread_create(&left_threads[i], NULL, left, (void*)&left_ids[i]);
    }

    for (int i = 0; i < right_cars; i++) {
        right_ids[i] = i + 1;
        pthread_create(&right_threads[i], NULL, right, (void*)&right_ids[i]);
    }

    for (int i = 0; i < left_cars; i++) {
        pthread_join(left_threads[i], NULL);
    }

    for (int i = 0; i < right_cars; i++) {
        pthread_join(right_threads[i], NULL);
    }

    sem_destroy(&bridge);
    sem_destroy(&mutex);

    return 0;
}
