#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <syslog.h>
#include <time.h>
#include "list.h"

/* List implementation */

typedef struct task {
    int hour;
    int minute;
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
    int cmpResult;
    if(task1->hour < task2->hour)
        return -1;
    if(task1->hour > task2->hour)
        return 1;
    
    if(task1->minute < task2->minute)
        return -1;
    if(task1->minute > task2->minute)
        return 1;

    cmpResult = strcmp(task1->command, task2->command);
    if(cmpResult != 0)
        return cmpResult;

    cmpResult = strcmp(task1->mode, task2->mode);
    return cmpResult;
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

    newTask->hour = atoi(hour);
    newTask->minute = atoi(minute);
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

void removeFirst() {
    tasklist *temp = taskListHead;

    if (taskListHead->next == NULL) {
        taskListHead = NULL;
        taskListTail = NULL;
    } else {
        taskListHead = temp->next;
        taskListHead->prev = NULL;
    }

    free(temp->task->command);
    free(temp->task->mode);
    free(temp->task);
    free(temp);
}

void removeAll() {
    while (taskListHead != NULL)
    {
        removeFirst();
    }
}

bool checkIfPast(int hour, int minute) {
    time_t currentTime = time(NULL);
    struct tm* localTime = localtime(&currentTime);
    int currentHour = localTime->tm_hour;
    int currentMinute = localTime->tm_min;

    if(hour < currentHour)
        return true;
    if(hour == currentHour && minute < currentMinute)
        return true;
    return false;
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

    if(checkIfPast(taskListTail->task->hour, taskListTail->task->minute))
        return;

    while (checkIfPast(taskListHead->task->hour, taskListHead->task->minute)) {
        i = taskListHead;
        taskListHead = taskListHead->next;
        taskListHead->prev = NULL;
        i->next = NULL;

        taskListTail->next = i;
        i->prev = taskListTail;
        taskListTail = i;
    }
}

void printTasksToSyslog() {
    tasklist *current = taskListHead;

    openlog(NULL, LOG_PID, LOG_USER);
    syslog(LOG_INFO, "Pozostałe zadania:\n");
    while (current != NULL)
    {
        task *currentTask = current->task;
        syslog(LOG_INFO, "%s:%s:%s:%s\n", toString(currentTask->hour), toString(currentTask->minute), currentTask->command, currentTask->mode);
        current = current->next;
    }
    closelog();
}

task* getFirstTask() {
    return taskListHead->task;
}

int listLength() {
    int length = 0;
    tasklist *current = taskListHead;

    while (current != NULL)
    {
        length++;
        current = current->next;
    }

    return length;
}

time_t getTimeToSleep(task *firstTask) {
    time_t currentTime = time(NULL);
    struct tm *currentLocalTime = localtime(&currentTime);
    struct tm taskLocalTime = {0};

    taskLocalTime.tm_hour = firstTask->hour;
    taskLocalTime.tm_min = firstTask->minute;
    taskLocalTime.tm_sec = 0;
    taskLocalTime.tm_year = currentLocalTime->tm_year;
    taskLocalTime.tm_mon = currentLocalTime->tm_mon;
    taskLocalTime.tm_mday = currentLocalTime->tm_mday;
    taskLocalTime.tm_isdst = -1;
    
    time_t taskTime = mktime(&taskLocalTime);
    if(taskTime - currentTime < 0)
        taskTime += 86400; // 1 day

    return taskTime - currentTime;
}

char* toString(int value) {
    char *bufor = malloc(sizeof(char) * 3);
    if(value < 10) {
        sprintf(bufor, "0%d", value);
    } else {
        sprintf(bufor, "%d", value);
    }
    return bufor;
}