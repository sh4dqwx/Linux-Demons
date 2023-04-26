#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>
#include <signal.h>

#include "list.h"

typedef struct task {
    int hour;
    int minute;
    char *command;
    char *mode;
} task;

void argValidation(int argc, char **argv) {
    if (argc != 3) {
        printf("Nie prawidłowy format: \"./minicron <taskfile> <outfile>\"\n");
        exit(EXIT_FAILURE);
    }
    
    if (strcmp(argv[1], argv[2]) == 0) {
        printf("Pliki \"taskfile\" i \"outfile\" nie mogą być tym samym plikiem\n");
        exit(EXIT_FAILURE);
    }

    if (access(argv[1], F_OK) < 0) {
        perror(argv[1]);
        exit(EXIT_FAILURE);
    }
    if (access(argv[2], F_OK) < 0) {
        perror(argv[2]);
        exit(EXIT_FAILURE);
    }
}

void readTaskFile(char *fileName) {
    FILE *taskFile = fopen(fileName, "r");
    char taskBufor[1000];
    while(fgets(taskBufor, sizeof(taskBufor), taskFile) != NULL) {
        int index = strlen(taskBufor)-1;
        while(index >= 0 && (taskBufor[index] == '\n' || taskBufor[index] == '\r')) {
            taskBufor[index] = '\0';
            index--;
        }
        char* taskString = malloc(strlen(taskBufor)+1);
        strcpy(taskString, taskBufor);
        addTask(taskString);
        free(taskString);
    }
    fclose(taskFile);
    sortTasks();
    printTasks();
}

void runTask(task *firstTask) {
    char *command = firstTask->command;
    char *mode = firstTask->mode;

    char *bufor;
    char *commands[100];
    int commandsCount = 0;
    bufor = strtok(command, "|");
    while (bufor != NULL) {
        commands[commandsCount] = bufor;
        bufor = strtok(NULL, "|");
        commandsCount++;
    }

    int pipes[commandsCount - 1][2];

    for (int i = 0; i < commandsCount - 1; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < commandsCount; i++) {
        bufor = strtok(commands[i], " ");
        char *args[100];
        int argsCount = 0;
        while (bufor != NULL) {
            args[argsCount] = bufor;
            bufor = strtok(NULL, " ");
            argsCount++;
        }
        args[argsCount] = NULL;

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid > 0) continue;
        
        if (i != 0) {
            dup2(pipes[i - 1][0], STDIN_FILENO);
        }

        int stdout_fd = open("/dev/stdout", O_WRONLY);
        int stderr_fd = open("/dev/stderr", O_WRONLY);

        if (i != commandsCount - 1) {
            dup2(pipes[i][1], STDOUT_FILENO);
        } else {
            printf("%s\n", mode);
            if(strcmp(mode, "0"))
                dup2(stdout_fd, STDOUT_FILENO);
            else if(strcmp(mode, "1"))
                dup2(stderr_fd, STDOUT_FILENO);
            else {
                dup2(STDOUT_FILENO, STDERR_FILENO);
                dup2(stdout_fd, STDOUT_FILENO);
                dup2(stderr_fd, STDOUT_FILENO);
            }
        }

        for (int j = 0; j < commandsCount - 1; j++) {
            close(pipes[j][0]);
            close(pipes[j][1]);
        }

        char commandName[100] = "/bin/";
        strcat(commandName, args[0]);
        execv(commandName, args);
        perror("execv");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < commandsCount - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for (int i = 0; i < commandsCount; i++) {
        wait(NULL);
    }
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
    argValidation(argc, argv);

    pid_t pid, sid;
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    umask(0);   
    sid = setsid();
    if (sid < 0) {
        perror("sid");
        exit(EXIT_FAILURE);
    }

    readTaskFile(argv[1]);

    while (listLength() > 0) {
        task *firstTask = getFirstTask();
        //removeTask(firstTask);
        time_t timeToSleep = getTimeToSleep(firstTask);
        sleep(5);
        
        pid_t task_pid = fork();
        if (task_pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (task_pid == 0) {
            runTask(firstTask);
        } else {
            wait(NULL);
        }
    }
    exit(EXIT_SUCCESS);
}