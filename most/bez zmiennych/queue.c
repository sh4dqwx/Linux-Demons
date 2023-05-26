#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct queueElement
{
    pthread_t threadId;
    int cityId;
    struct queueElement *next;
} queueElement;

void queueAdd(queueElement** head, pthread_t id, int cityId) {
    queueElement* newElement = (queueElement*)malloc(sizeof(queueElement));
    newElement->threadId = id;
    newElement->cityId = cityId;
    newElement->next = NULL;

    if (*head == NULL) {
        *head = newElement;
        return;
    }

    queueElement* current = *head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = newElement;
}

queueElement* queuePop(queueElement** head) {
    if (*head == NULL) {
        return NULL;
    }

    queueElement* current = *head;
    *head = (*head)->next;
    return current;
}

void queueRemoveFirst(queueElement** head) {
    if (*head == NULL) {
        return;
    }

    queueElement* current = *head;
    *head = (*head)->next;
    free(current);
}

void queuePrint(queueElement* head) {
    queueElement* current = head;
    while (current != NULL) {
        printf("Thread id: %ld, city id: %d\n", current->threadId, current->cityId);
        current = current->next;
    }
}

int queueSize(queueElement* head) {
    int size = 0;
    queueElement* current = head;
    while (current != NULL) {
        size++;
        current = current->next;
    }
    return size;
}
