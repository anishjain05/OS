#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

void display_time(const char *format, const struct tm *timeinfo) {
    char buffer[100];
    strftime(buffer, sizeof(buffer), format, timeinfo);
    printf("%s\n", buffer);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s [-d STRING | -R] file_name\n", argv[0]);
        return 1;
    }

    if (argc == 2){
        argc = 3;
        argv[2] = argv[1];
        argv[1] = "-d";
    }

    char *option = argv[1];
    char *file_name = argv[2];
    

    struct stat file_info;
    if (stat(file_name, &file_info) != 0) {
        perror("Error");
        return 1;
    }
    

    const struct tm *timeinfo = localtime(&file_info.st_mtime);

    

    if (strcmp(option, "-d") == 0 && argc > 3) {
        const char *format_string = argv[3];
        display_time(format_string, timeinfo);
    } else if (strcmp(option, "-R") == 0) {
        display_time("%a, %d %b %Y %H:%M:%S %z", timeinfo);
    } else {
        display_time("%c", timeinfo);
    }
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
        printf("Custom Shell ~%s ", cwd);
    else
        perror("getcwd");
    return 0;
}
