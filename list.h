#ifndef LIST_H
#define LIST_H

typedef struct task task;

task* createTask(char* taskName);
void addTask(char* taskString);
void removeTask(task* task);
void sortTasks();
void printTasks();
task* getFirstTask();
int listLength();

#endif