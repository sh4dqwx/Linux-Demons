#ifndef QUEUE_H
#define QUEUE_H

typedef struct queueElement queueElement;

void queueAdd(queueElement **head, pthread_t threadId, int cityId);
queueElement *queuePop(queueElement **head);
void queueRemoveFirst(queueElement **head);
int queueSize(queueElement *head);

#endif