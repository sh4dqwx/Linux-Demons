#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include "currentTime.h"
#include "queue.h"

queueElement *city1_queue;
queueElement *city2_queue;
pthread_t bridge;

void initBridge(int n) {
    city1_queue = malloc(sizeof(pthread_t) * n);
    city2_queue = malloc(sizeof(pthread_t) * n);
}

int leaveCity(pthread_t threadid, int cityId, currentTime time) {
    if (cityId == 1) {
        queueAdd(city1_queue, threadid, currentTime);
        return 2;
    } else {
        queueAdd(city2_queue, threadid, currentTime);
        return 1;
    }
}

void arbiter() {
    // while (1)
    // {
    //     if ()
    // }
}