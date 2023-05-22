#include <stdio.h>
#include <stdlib.h>
#include "currentTime.h"

struct queueElement {
    pthread_t id;
    currentTime time;
    struct queueElement* next;
} typedef queueElement;

void queueAdd(queueElement** head, pthread_t id, currentTime time) {
    queueElement* newElement = malloc(sizeof(queueElement));
    newElement->id = id;
    newElement->time = time;
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

void queueRemove(queueElement** head) {
    if (*head == NULL) {
        return;
    }

    queueElement* temp = *head;
    *head = (*head)->next;
    free(temp);
}

void queuePrint(queueElement* head) {
    queueElement* current = head;
    while (current != NULL) {
        printf("Thread ID: %ld, Time: %ld:%ld\n", current->id, current->time.seconds, current->time.miliseconds);
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