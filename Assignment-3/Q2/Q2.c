#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

int P, C, loaded_passengers = 0, remaining_passengers;
sem_t mutex, board_queue, unboard_queue;

void* car(void* args) {
    while (remaining_passengers > 0) {
        printf("Begin loading\n");
        load();

        for (int i = 0; i < C; ++i) {
            sem_post(&board_queue); 
            sem_wait(&mutex);       
            printf("Passenger %d boarded\n", loaded_passengers - C + i + 1);
        }

        printf("Run the duration of the ride\n");
        sleep(2); 

        printf("Begin unloading\n");
        unload();

        for (int i = 0; i < C; ++i) {
            sem_post(&unboard_queue); 
            sem_wait(&mutex);         
            printf("Passenger %d got off\n", loaded_passengers - C + i + 1);
            remaining_passengers--;
        }
    }

    pthread_exit(NULL);
}

// Passenger thread
void* passenger(void* args) {
    int id = (int)args;

    while (1) {
        
        sem_wait(&board_queue);

        
        if (remaining_passengers <= 0) {
            sem_post(&mutex); 
            break; 
        }

        board();
        sem_post(&mutex); 

        sem_wait(&unboard_queue);
        offboard();
        sem_post(&mutex); 
    }

    pthread_exit(NULL);
}

void load() {
    sleep(1);
    loaded_passengers += C;
}

void unload() {
    sleep(1);
}

void board() {
    sleep(1);
}

void offboard() {
    sleep(1);
}

int main() {
    printf("Enter the total number of passengers (P): ");
    scanf("%d", &P);
    printf("Enter the car capacity (C): ");
    scanf("%d", &C);

    if (C >= P) {
        printf("Error: Car capacity should be less than total passengers.\n");
        return 1;
    }

    remaining_passengers = P;

    sem_init(&mutex, 0, 1);
    sem_init(&board_queue, 0, 0);
    sem_init(&unboard_queue, 0, 0);

    pthread_t car_thread;
    pthread_create(&car_thread, NULL, car, NULL);

    pthread_t passenger_threads[P];
    int passenger_ids[P];
    for (int i = 0; i < P; ++i) {
        passenger_ids[i] = i + 1;
        pthread_create(&passenger_threads[i], NULL, passenger, (void*)&passenger_ids[i]);
    }

    pthread_join(car_thread, NULL);

    for (int i = 0; i < P; ++i) {
        pthread_join(passenger_threads[i], NULL);
    }

    sem_destroy(&mutex);
    sem_destroy(&board_queue);
    sem_destroy(&unboard_queue);

    return 0;
}