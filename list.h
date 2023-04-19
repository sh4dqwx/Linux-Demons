#ifndef LIST_H
#define LIST_H

typedef struct task task;
typedef struct tasklist tasklist;

task* createTask(char* taskName);
void addTask(char* taskString);
void removeTask(task* task);
void sortTasks();
void printTasks();
tasklist* getTaskListHead();

#endif