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

#include "list.h"

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
    char taskBufor[100];
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

time_t getTimeToSleep(task *firstTask) {
    time_t currentTime = time(NULL);
    struct tm taskLocalTime = {0};
    taskLocalTime.tm_hour = firstTask->hour;
    taskLocalTime.tm_min = firstTask->minute;
    time_t taskTime = mktime(&taskLocalTime);

    return taskTime - currentTime;
}

void runTask(task *firstTask) {
    // char *command = firstTask->command;
    // char *args[100];
    // int i = 0;
    // args[i] = strtok(command, " ");
    // while(args[i] != NULL) {
    //     i++;
    //     args[i] = strtok(NULL, " ");
    // }
    // args[i] = NULL;
    // execvp(args[0], args);
    // perror("execvp");
    // exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    /* Our args validation function */
    argValidation(argc, argv);

    /* Our process ID and Session ID */
    pid_t pid, sid;
    
    /* Fork off the parent process */
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    /* If we got a good PID, then
        we can exit the parent process. */
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    /* Change the file mode mask */
    umask(0);
            
    /* Open any logs here */        
            
    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0) {
        /* Log the failure */
        exit(EXIT_FAILURE);
    }
    
    /* Close out the standard file descriptors */
    //close(STDIN_FILENO);
    //close(STDOUT_FILENO);
    //close(STDERR_FILENO);
    
    /* Daemon-specific initialization goes here */

    /* Opening files */
    readTaskFile(argv[1]);
    exit(EXIT_SUCCESS);
    /* The Big Loop */
    while (listLength() > 0) {
        task *firstTask = getFirstTask();
        time_t timeToSleep = getTimeToSleep(firstTask);
        sleep(timeToSleep);
        
        pid_t task_pid = fork();
        if (task_pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (task_pid == 0) 
            runTask(firstTask);
    }
    exit(EXIT_SUCCESS);
}