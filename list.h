#ifndef LIST_H
#define LIST_H

typedef struct task task;

task* createTask(char* taskName);
void addTask(char* taskString);
void removeFirst();
void removeAll();
void sortTasks();
void printTasks();
void printTasksToSyslog();
task* getFirstTask();
int listLength();
time_t getTimeToSleep(task *firstTask);
char* toString(int value);

#endif