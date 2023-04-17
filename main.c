#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>

#include "list.h"

void readFromFile(char *fileName) {
        FILE *taskFile = fopen(fileName, "r");
        if(taskFile == NULL) {
                perror("ERROR");
                exit(EXIT_FAILURE);
        }

        char singleTask[1000];
        while(fgets(singleTask, sizeof(singleTask), taskFile) != NULL) {
                addTask(singleTask);
        }
        fclose(taskFile);
        printf("Wczytano plik");
        //tasklist* task = getTaskListHead();
        //printf("%s:%s:%s:%s", task->task->hour, task->task->minute, task->task->command, task->task->mode);

        printTasks();
}

void argValidation(int argc, char **argv) {
        if (argc != 3) {
                printf("Nie prawidłowy format: \"./minicron <taskfile> <outfile>\"\n");
                exit(EXIT_FAILURE);
        }
        
        if (strcmp(argv[1], argv[2]) == 0) {
                printf("Pliki \"taskfile\" i \"outfile\" nie mogą być tym samym plikiem\n");
                exit(EXIT_FAILURE);
        }
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
        close(STDIN_FILENO);
        //close(STDOUT_FILENO);
        close(STDERR_FILENO);
        
        /* Daemon-specific initialization goes here */

        /* Opening files */
        readFromFile(argv[1]);
        sortTasks();
        exit(EXIT_SUCCESS);
        //sortTasks(taskFile);
        /* The Big Loop */
        while (1) {
           /* Do some task here ... */
                sleep(10); /* wait 30 seconds */
        }
   exit(EXIT_SUCCESS);
}