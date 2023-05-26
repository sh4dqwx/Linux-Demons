#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include "queue.h"

typedef struct queueElement
{
    pthread_t threadId;
    int cityId;
    struct queueElement *next;
} queueElement;

typedef struct carListElement
{
    pthread_t threadId;
    int cityId;
    int queueId;
    pthread_cond_t carVar;
} carListElement;

bool info = false;
queueElement *cityQueueHead = NULL;
pthread_mutex_t queueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t carOnBridge;
carListElement *carList;
int carListCount = 0;

void initBridge(int maxCars, bool infoFlag)
{
    carList = malloc(sizeof(carListElement) * maxCars);
    info = infoFlag;
}

void addCarToCity(pthread_t threadId, int cityId)
{
    carList[carListCount].threadId = threadId;
    carList[carListCount].cityId = cityId;
    carList[carListCount].queueId = 0;
    pthread_cond_init(&carList[carListCount].carVar, NULL);
    carListCount++;
}

void printCarList() {
    for(int i=0; i<carListCount; i++) {
        printf("%d: Samochód %ld\n", i, carList[i].threadId);
        printf("Miasto - %d\n", carList[i].cityId);
        printf("Kolejka - %d\n\n", carList[i].queueId);
    }
}

int countCarsInCity(int cityId)
{
    int count = 0;
    for (int i = 0; i < carListCount; i++)
    {
        if (carList[i].cityId != cityId)
            continue;
        count++;
    }
    return count;
}

int countCarsInQueue(int queueId)
{
    int count = 0;
    for (int i = 0; i < carListCount; i++)
    {
        if (carList[i].queueId != queueId)
            continue;
        count++;
    }
    return count;
}

void printBridgeState()
{
    if (carOnBridge == 0)
        printf("A-%d %d --> [   -   ] <-- %d %d-B\n",
               countCarsInCity(1),
               countCarsInQueue(1),
               countCarsInQueue(2),
               countCarsInCity(2));
    else if (carList[carOnBridge - 1].cityId == -1)
        printf("A-%d %d --> [>> %ld >>] <-- %d %d-B\n",
               countCarsInCity(1),
               countCarsInQueue(1),
               carOnBridge,
               countCarsInQueue(2),
               countCarsInCity(2));
    else
        printf("A-%d %d --> [<< %ld <<] <-- %d %d-B\n",
               countCarsInCity(1),
               countCarsInQueue(1),
               carOnBridge,
               countCarsInQueue(2),
               countCarsInCity(2));
        
}

void printAllDetails()
{
    int carsInCityA = countCarsInCity(1);
    int carsInCityB = countCarsInCity(2);
    int carsInQueueA = countCarsInQueue(1);
    int carsInQueueB = countCarsInQueue(2);

    printf("-----------------------------------------------------\n");
    printf("Miasto A:\n- samochody w miescie (%d):\n", carsInCityA);
    for (int i = 0; i < carListCount; i++)
    {
        if (carList[i].cityId != 1)
            continue;
        printf("  - %ld\n", carList[i].threadId);
    }

    printf("- samochody w kolejce (%d):\n", carsInQueueA);
    for (int i = 0; i < carListCount; i++)
    {
        if (carList[i].queueId != 1)
            continue;
        printf("  - %ld\n", carList[i].threadId);
    }

    printf("Samochod na moście: (%d)\n", carOnBridge != 0 ? 1: 0);
    if (carOnBridge != 0)
        printf("  - %ld\n", carOnBridge);

    printf("Miasto B:\n- samochody w kolejce (%d):\n", carsInQueueB);
    for (int i = 0; i < carListCount; i++)
    {
        if (carList[i].queueId != 2)
            continue;
        printf("  - %ld\n", carList[i].threadId);
    }

    printf("- samochody w miescie (%d):\n", carsInCityB);
    for (int i = 0; i < carListCount; i++)
    {
        if (carList[i].cityId != 2)
            continue;
        printf("  - %ld\n", carList[i].threadId);
    }
}

int leaveCity(pthread_t threadId, int cityId)
{
    pthread_mutex_lock(&queueMutex);
    queueAdd(&cityQueueHead, threadId, cityId);
    carList[threadId - 1].cityId = 0 - cityId;
    carList[threadId - 1].queueId = cityId;
    if (info)
        printAllDetails();
    else
        printBridgeState();

    pthread_cond_wait(&carList[threadId - 1].carVar, &queueMutex);
    
    carList[threadId - 1].queueId = 0;

    if (info)
        printAllDetails();
    else
        printBridgeState();

    pthread_mutex_unlock(&queueMutex);

    sleep(1); // Czas przejazdu przez most

    pthread_mutex_lock(&queueMutex);
    carList[threadId - 1].cityId = (cityId == 1 ? 2 : 1);
    carOnBridge = 0;
    if (info)
        printAllDetails();
    else
        printBridgeState();
    pthread_mutex_unlock(&queueMutex);
    return carList[threadId - 1].cityId;
}

void arbiter()
{
    pthread_mutex_lock(&queueMutex);
    if(carOnBridge != 0) {
        pthread_mutex_unlock(&queueMutex);
        return;
    }

    queueElement *firstCarInQueueToBridgeBecauseWhyNot = queuePop(&cityQueueHead);
    pthread_mutex_unlock(&queueMutex);

    if(firstCarInQueueToBridgeBecauseWhyNot == NULL) {
        return;
    }

    carOnBridge = firstCarInQueueToBridgeBecauseWhyNot->threadId;
    pthread_cond_signal(&carList[firstCarInQueueToBridgeBecauseWhyNot->threadId - 1].carVar);

    free(firstCarInQueueToBridgeBecauseWhyNot);
}
