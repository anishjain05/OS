#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
int main(int argc, char *argv[]) {
    int remove_existing = 0;
    int verbose = 0;
    char *dir_name = NULL;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-r") == 0)
            remove_existing = 1;
        else if (strcmp(argv[i], "-v") == 0)
            verbose = 1;
        else {
            dir_name = argv[i];
            break;
        }
    }
    if (dir_name == NULL) {
        printf("Usage: %s [-r] [-v] dir_name\n", argv[0]);
        return 1;
    }
    pid_t rc = fork();
    if (rc < 0)
        perror("fork");
    else if (rc) {
        int child_status;
        waitpid(rc, &child_status, 0);
        if (WIFEXITED(child_status)) {
            if (verbose)
                printf("Child process exited with status: %d\n", WEXITSTATUS(child_status));
            
            if (chdir(dir_name) == -1) {
                perror("chdir");
                return 1;
            }
            else if (verbose != 0)
                printf("Changed to directory: %s\n", dir_name);
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) != NULL)
                printf("Current working directory: %s\n", cwd);
            else
                perror("getcwd");
            if (chdir(cwd) == 0) {
                printf("Custom shell ~%s",cwd);
            } else {
                perror("chdir");
    }
        }
        else {
            perror("waitpid");
            return 1;
        }
    }
    else {
        if (remove_existing && rmdir(dir_name) == -1) {
            perror("rmdir");
            exit(1);
        }
        else if (mkdir(dir_name, 0777) == -1) {
            perror("mkdir");
            exit(1);
        }
        else if (verbose != 0)
            printf("Created directory: %s\n", dir_name);
        exit(0);
    }
    return 0;
}

