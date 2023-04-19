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

tasklist* taskListHead = NULL;
tasklist* taskListTail = NULL;

int compareTasks(task* task1, task* task2) {
    int cmpResult = strcmp(task1->hour, task2->hour);
    if(cmpResult != 0) {
        return cmpResult;
    } else {
        cmpResult = strcmp(task1->minute, task2->minute);
        if(cmpResult != 0) {
            return cmpResult;
        } else {
            cmpResult = strcmp(task1->command, task2->command);
            if(cmpResult != 0) {
                return cmpResult;
            } else {
                cmpResult = strcmp(task1->mode, task2->mode);
                return cmpResult;
            }
        }
    }
}

task* createTask(char* taskName) {
    char taskElementBufor[100]; int index = 0;
    char* hour = NULL; char* minute = NULL;
    char* command = NULL; char* mode = NULL;
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
            }

            while(index > 0) {
                index--;
                taskElementBufor[index] = '\0';
            }
        } else {
            taskElementBufor[index] = taskName[i];
            index++;
        }
    }
    mode = malloc(index+2);
    strcpy(mode, taskElementBufor);

    task* newTask = malloc(sizeof(task));

    newTask->hour = malloc(strlen(hour)+1);
    strcpy(newTask->hour, hour);
    newTask->minute = malloc(strlen(minute)+1);
    strcpy(newTask->minute, minute);
    newTask->command = malloc(strlen(command)+1);
    strcpy(newTask->command, command);
    newTask->mode = malloc(strlen(mode)+1);
    strcpy(newTask->mode, mode);

    return newTask;
}

void addTask(char* taskString) {
    tasklist *newTaskList = malloc(sizeof(tasklist));
    newTaskList->task = createTask(taskString);
    newTaskList->next = NULL;
    newTaskList->prev = taskListTail;

    if (taskListHead == NULL) {
        taskListHead = newTaskList;
        taskListTail = newTaskList;
    } else {
        taskListTail->next = newTaskList;
        taskListTail = newTaskList;
    }
}

void removeTask(task* task) {
    if (taskListHead == NULL)
        return;

    tasklist *current = taskListHead;

    while (current != NULL) {
        if (compareTasks(current->task, task) == 0) break;
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

    free(current->task->hour);
    free(current->task->minute);
    free(current->task->command);
    free(current->task->mode);
    free(current->task);
    free(current);
}

void sortTasks() {
    if (taskListHead == NULL)
        return;

    tasklist* i = taskListHead;
    tasklist* j = NULL;
    task* temp = NULL;

    while (i != NULL) {
        j = i->next;
        while (j != NULL) {
            if (compareTasks(i->task, j->task) > 0) {
                temp = i->task;
                i->task = j->task;
                j->task = temp;
            }
            j = j->next;
        }
        i = i->next;
    }
}

// Nie usuwaj!! poprawie i bedzie git
// void quickSort(tasklist* L, tasklist* P) {
//     if(L->index == P->index) return;

//     tasklist* i = L; tasklist* j = P->next;
//     do {
//         do i = i->next; while(i->index <= P->index && compareTasks(i->task, L->task) < 0);
//         do {
//             if(j == NULL) j = taskListTail;
//             else j = j->prev;
//         } while(compareTasks(j->task, L->task) > 0);
//         if(i->index < j->index) {
//             task* tmp = i->task;
//             i->task = j->task;
//             j->task = tmp;
//         }
//     } while(i->index < j->index);
//     task* tmp = L->task;
//     L->task = j->task;
//     j->task = tmp;

//     sortTasks(L, j->prev);
//     sortTasks(j->next, P);
// }

// void sortTasks() {
//     quickSort(taskListHead, taskListTail);
// }

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