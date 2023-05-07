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
#include <sys/select.h>
#include <signal.h>

#include "list.h"

volatile sig_atomic_t exitFlag = 1;
char *taskfileName;

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
}

void runTask(task *firstTask, char *outfileName) {
    char *commandString = firstTask->command;
    char *mode = firstTask->mode;

    char taskBufor[1000];
    sprintf(taskBufor, "\n%s:%s:%s:%s\n", toString(firstTask->hour), toString(firstTask->minute), firstTask->command, firstTask->mode);
    int outfileFd = open(outfileName, O_WRONLY | O_APPEND);
    int nullFd = open("/dev/null", O_WRONLY);
    if(write(outfileFd, taskBufor, strlen(taskBufor)) < 0) {
        perror("write");
        exit(EXIT_FAILURE);
    }

    char *bufor;
    char *commands[100];
    int commandsCount = 0;
    bufor = strtok(commandString, "|");
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

        if (i != commandsCount - 1) {
            dup2(pipes[i][1], STDOUT_FILENO);
        } else {
            if(!strcmp(mode, "1")) {
                dup2(nullFd, STDOUT_FILENO);
            } else {
                dup2(outfileFd, STDOUT_FILENO);
            }
        }

        for (int j = 0; j < commandsCount - 1; j++) {
            close(pipes[j][0]);
            close(pipes[j][1]);
        }

        if(!strcmp(mode, "0")) {
            close(STDERR_FILENO);
        } else {
            dup2(outfileFd, STDERR_FILENO);
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

    int pipeStatus = 0;
    for (int i = 0; i < commandsCount; i++) {
        int commandStatus;
        wait(&commandStatus);
        int exitCode = WEXITSTATUS(commandStatus);
        if (pipeStatus == 0) {
            pipeStatus = exitCode;
        }
    }

    close(nullFd);
    close(outfileFd);
    exit(pipeStatus);
}

void sigintHandler(int signum) {
    exitFlag = 0;
}

void sigusr1Handler(int sigum) {
    exitFlag = -1;

    removeAll();
    readTaskFile(taskfileName);
    sortTasks();
    exitFlag = -2;
}

void sigusr2Handler(int signum) {
    printTasksToSyslog();
}

int main(int argc, char **argv) {
    argValidation(argc, argv);
    taskfileName = argv[1];

    pid_t pid, sid;
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    signal(SIGINT, sigintHandler);
    signal(SIGUSR1, sigusr1Handler);
    signal(SIGUSR2, sigusr2Handler);

    umask(0);   
    sid = setsid();
    if (sid < 0) {
        perror("sid");
        exit(EXIT_FAILURE);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    readTaskFile(taskfileName);
    sortTasks();

    while (listLength() > 0) {
        task *firstTask = getFirstTask();
        time_t timeToSleep = getTimeToSleep(firstTask);
        for (time_t i = 0; i < timeToSleep; i++)
        {
            sleep(1);
            if(exitFlag == 0) {
                exit(EXIT_SUCCESS);
            } else if (exitFlag < 0) {
                break;
            }
        }
        if (exitFlag < 0) {
            while (exitFlag == -1) {
                sleep(1);
            }
            exitFlag = 1;
            continue;
        }

        int resultCode;
        pid_t task_pid = fork();
        if (task_pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (task_pid == 0) {
            runTask(firstTask, argv[2]);
        } else {
            openlog(NULL, LOG_PID, LOG_USER);
            syslog(LOG_INFO, "Rozpoczęcie zadania: %s:%s:%s:%s", toString(firstTask->hour), toString(firstTask->minute), firstTask->command, firstTask->mode);
            closelog();

            int status;
            wait(&status);
            int resultCode = WEXITSTATUS(status);
            
            openlog(NULL, LOG_PID, LOG_USER);
            syslog(LOG_INFO, "Kod wyjścia zadania: %d", resultCode);
            closelog();

            removeFirst();
        }
    }
    exit(EXIT_SUCCESS);
}