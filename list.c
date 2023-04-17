#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "list.h"

/* List implementation */

typedef struct task {
    char *hour;
    char *minute;
    char *command;
    char *mode;
} task;

typedef struct tasklist {
    task *task;
    struct tasklist *next;
    struct tasklist *prev;
} tasklist;

tasklist *taskListHead = NULL;
tasklist *taskListTail = NULL;

bool compareTasks(task* task1, task* task2) {
    return strcmp(task1->hour, task2->hour) == 0 &&
           strcmp(task1->minute, task2->minute) == 0 &&
           strcmp(task1->command, task2->command) == 0 &&
           strcmp(task1->mode, task2->mode) == 0;
}

task* createTask(char* taskName) {
    char taskElementBufor[1000];
    int index = 0;
    char *hour = NULL;
    char *minute = NULL;
    char *command = NULL;
    char *mode = NULL;
    for(int i=0; i<strlen(taskName); i++) {
        if(taskName[i] == ':') {
            if(hour == NULL) {
                hour = malloc(index+2);
                strcpy(hour, taskElementBufor);
            } else if(minute == NULL) {
                minute = malloc(index+2);
                strcpy(minute, taskElementBufor);
            } else if(command == NULL) {
                command = malloc(index+2);
                strcpy(command, taskElementBufor);
            } else if(mode == NULL) {
                mode = malloc(index+2);
                strcpy(mode, taskElementBufor);
            }
            index = 0;
        } else {
            taskElementBufor[index] = taskName[i];
            index++;
        }
    }
    task* newTask = malloc(sizeof(task));
    strcpy(newTask->hour, hour);
    strcpy(newTask->minute, minute);
    strcpy(newTask->command, command);
    strcpy(newTask->mode, mode);
    return newTask;
}

void addTask(char* task) {
    tasklist *newTask = malloc(sizeof(tasklist));
    newTask->task = createTask(task);
    newTask->next = NULL;
    newTask->prev = taskListTail;

    if (taskListHead == NULL) {
        taskListHead = newTask;
        taskListTail = newTask;
    } else {
        taskListTail->next = newTask;
        taskListTail = newTask;
    }
}

void removeTask(task* task) {
    if (taskListHead == NULL)
        return;

    tasklist *current = taskListHead;

    while (current != NULL) {
        if (compareTasks(current->task, task)) break;
        else current = current->next;
    }

    if (current == NULL)
        return;
    
    tasklist *prev = current->prev;
    tasklist *next = current->next;

    if (prev == NULL ) {
        taskListHead = next;
    } else {
        prev->next = next;
    }
    
    if (next == NULL) {
        taskListTail = prev;
    } else {
        next->prev = prev;
    }

    free(current->task);
    free(current);
}

void sortTasks() {
    if (taskListHead == NULL)
        return;

    tasklist *current = taskListHead;
    tasklist *next = NULL;
    task *temp = NULL;

    while (current != NULL) {
        next = current->next;
        while (next != NULL) {
            if ( current->task->hour > next->task->hour && current->task->minute > next->task->minute ) {
                temp = current->task;
                current->task = next->task;
                next->task = temp;
            }
            next = next->next;
        }
        current = current->next;
    }
}

void printTasks() {
    tasklist *current = taskListHead;

    while (current != NULL)
    {
        task *currentTask = current->task;
        printf("%s:%s:%s:%s\n", currentTask->hour, currentTask->minute, currentTask->command, currentTask->mode);
        current = current->next;
    }
}

tasklist* getTaskListHead() {
    return taskListHead;
}