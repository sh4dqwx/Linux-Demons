#ifndef QUEUE_H
#define QUEUE_H

typedef struct queueElement queueElement;

void queueAdd(queueElement **head, pthread_t id, currentTime time);
void queueRemove(queueElement **head);
void queuePrint(queueElement *head);
int queueSize(queueElement *head);

#endif