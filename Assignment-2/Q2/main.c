#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/wait.h>
#include <math.h>
#include <time.h>

void counting_task() {
    unsigned long long count = 1;
    unsigned long long count_to = pow(2, 32);
    while (count < count_to) {
        count++;
    }
}

int main() {
    int status;
    pid_t child_pid1, child_pid2, child_pid3;
    struct timespec start_time, end_time;
    double exec_time1, exec_time2, exec_time3;

    FILE *file = fopen("execution_times.txt", "w");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    // Process 3: FIFO 
    child_pid3 = fork();
    if (child_pid3 == 0) {
        struct sched_param param;
        param.sched_priority = 1;
        sched_setscheduler(0, SCHED_FIFO, &param);
        clock_gettime(CLOCK_MONOTONIC, &start_time);
        counting_task();
        clock_gettime(CLOCK_MONOTONIC, &end_time);
        exec_time3 = (end_time.tv_sec - start_time.tv_sec) +
                     (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
        fprintf(file, "SCHED_FIFO %.2f\n", exec_time3);
        fclose(file);
        exit(0);
    }

    // Process 2: RR 
    child_pid2 = fork();
    if (child_pid2 == 0) {
        // struct sched_param param;
        // param.sched_priority = 1; 
        sched_setscheduler(0, SCHED_RR, NULL);
        clock_gettime(CLOCK_MONOTONIC, &start_time);
        counting_task();
        clock_gettime(CLOCK_MONOTONIC, &end_time);
        exec_time2 = (end_time.tv_sec - start_time.tv_sec) +
                     (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
        fprintf(file, "SCHED_RR %.2f\n", exec_time2);
        fclose(file);
        exit(0);
    }
    
    // Process 1: OTHER
    child_pid1 = fork();
    if (child_pid1 == 0) {
        struct sched_param param;
        param.sched_priority = 0; 
        sched_setscheduler(0, SCHED_OTHER, &param);
        clock_gettime(CLOCK_MONOTONIC, &start_time);
        counting_task();
        clock_gettime(CLOCK_MONOTONIC, &end_time);
        exec_time1 = (end_time.tv_sec - start_time.tv_sec) +
                     (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
        fprintf(file, "SCHED_OTHER %.2f\n", exec_time1);
        fclose(file);
        exit(0);
    }

    // Waiting for all child processes to finish
    waitpid(child_pid1, &status, 0);
    waitpid(child_pid2, &status, 0);
    waitpid(child_pid3, &status, 0);

    return 0;
}



