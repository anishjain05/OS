#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#define MAX_INPUT_LENGTH 100
/*int execute_command(char *command) {
    pid_t pid = fork();
    
    if (pid == 0) { // Child process
        char *args[] = {"./dir", command, NULL}; // Assuming your compiled custom_commands executable is named custom_commands
        execvp(args[0], args);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else if (pid > 0) { // Parent process
        wait(NULL);
    } else { // Fork failed
        perror("fork");
    }

    return 1;
}*/
int count_words(FILE *file, int ignore_newline) {
    int word_count = 0;
    int in_word = 0;
    char c;
    while ((c = fgetc(file)) != EOF) {
        if (ignore_newline && c == '\n')
            continue;
        if (c == ' ' || c == '\t' || c == '\n') {
            if (in_word) {
                in_word = 0;
                word_count++;
            }
        } else
            in_word = 1;
    }
    if (in_word)
        word_count++;
    return word_count;
}

int execute_word(int argc, char *argv[]) {
    int ignore_newline = 0;
    int compare_mode = 0;
    int file_count = 0;
    if (argc < 2) {
        printf("Usage: %s [-n] [-d] [file1] [file2]\n", argv[0]);
        return 1;
    }
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0)
            ignore_newline = 1;
        else if (strcmp(argv[i], "-d") == 0)
            compare_mode = 1;
        else
            file_count++;
    }
    if (file_count != 1 && !compare_mode) {
        printf("Error: You must provide exactly one file unless in compare mode.\n");
        return 1;
    }
    FILE *file1 = fopen(argv[argc - 1], "r");
    if (!file1) {
        perror("Error");
        return 1;
    }
    int word_count1 = count_words(file1, ignore_newline);
    printf("Word count in %s: %d\n", argv[argc - 1], word_count1);
    fclose(file1);
    if (compare_mode && argc == 4) {
        FILE *file2 = fopen(argv[argc - 2], "r");
        if (!file2) {
            perror("Error");
            return 1;
        }
        int word_count2 = count_words(file2, ignore_newline);
        printf("Word count in %s: %d\n", argv[argc - 2], word_count2);
        fclose(file2);
        printf("Difference in word count: %d\n", abs(word_count1 - word_count2));
    }
    return 0;
}
int execute_dir(char *input[], int word_count) {
    pid_t pid = fork();
    if (pid == 0) {
        char executable_path[100];
        snprintf(executable_path, sizeof(executable_path), "./%s", input[0]);
        input[word_count] = NULL;
        execvp(executable_path, input);
        perror("execvp");
        exit(EXIT_FAILURE);
    } 
    else if (pid > 0)
        wait(NULL);
    else
        perror("fork");
    return 1;
}
int execute_date(char *input[], int word_count) {
    pid_t pid = fork();
    if (pid == 0) {
        char executable_path[100];
        snprintf(executable_path, sizeof(executable_path), "./%s", input[0]);
        input[word_count] = NULL;
        execvp(executable_path, input);
        perror("execvp");
        exit(EXIT_FAILURE);
    } 
    else if (pid > 0)
        wait(NULL);
    else
        perror("fork");
    return 1;
}

int main() {
    int result;
    char user_input[MAX_INPUT_LENGTH];
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
        printf("Custom Shell ~%s ", cwd);
    else
        perror("getcwd");
    while (1) {
        fgets(user_input, sizeof(user_input), stdin);
        user_input[strcspn(user_input, "\n")] = '\0';
        char *input[100];
        int word_count = 0;
        char *token = strtok(user_input, " "); // Split the input using spaces as the delimiter
        while (token != NULL && word_count < 100) {
            input[word_count] = token;
            word_count++;
            token = strtok(NULL, " "); // Continue splitting the input
        }
        if (strcmp(input[0], "exit") == 0) {
            printf("Exiting Custom Shell.\n");
            break;
        }
        else if (strcmp(input[0], "dir") == 0) {
            //printf("Entered if loop\n");
            result = execute_dir(input, word_count);
            if (result == -1) {
                printf("Error executing command.\n");
            }
        }
        else if (strcmp(input[0], "date") == 0) {
            //printf("Entered if loop\n");
            result = execute_date(input, word_count);
            if (result == -1) {
                printf("Error executing command.\n");
            }
        }
        else if (strcmp(input[0], "word") == 0) {
            //printf("Entered if loop\n");
            result = execute_word(word_count, input);
            if (getcwd(cwd, sizeof(cwd)) != NULL){
                printf("Custom Shell ~%s ", cwd);
            }
            else{
                perror("getcwd");
            }
            if (result == -1) {
                printf("Error executing command.\n");
            }
        }

        else {
            printf("Not a valid command!");
            return 0;
        }
        // Execute the custom command
        /*result = execute_command(input);
        if (result == -1) {
            printf("Error executing command.\n");
        }*/
    }
    return 0;
}