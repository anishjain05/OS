#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
    pid_t child_pid;

    child_pid = fork();
    
    if (child_pid < 0) {
        perror("Fork failed");
        return 1;
    } else if (child_pid == 0) {
        printf("Child is having ID %d\n", getpid());
        printf("My Parent ID is %d\n", getppid());
    } else {
        printf("Parent (P) is having ID %d\n", getpid());
        
        wait(NULL);
        
        printf("ID of P's Child is %d\n", child_pid);
    }

    return 0;
}
