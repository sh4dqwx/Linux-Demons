#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
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
} carListElement;

bool info = false;
queueElement *cityQueueHead = NULL;
sem_t queueSemaphore;
pthread_t carOnBridge;
carListElement *carList;
int carListCount = 0;

void initBridge(int maxCars, bool infoFlag)
{
    if (sem_init(&queueSemaphore, 0, 1) != 0) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    carList = malloc(sizeof(carListElement) * maxCars);
    if(carList == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    info = infoFlag;
}

void addCarToCity(pthread_t threadId, int cityId)
{
    carList[carListCount].threadId = threadId;
    carList[carListCount].cityId = cityId;
    carList[carListCount].queueId = 0;
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
    if(sem_wait(&queueSemaphore)) {
        perror("sem_wait");
        exit(EXIT_FAILURE);
    }
    queueAdd(&cityQueueHead, threadId, cityId);
    carList[threadId - 1].cityId = 0 - cityId;
    carList[threadId - 1].queueId = cityId;
    if (info)
        printAllDetails();
    else
        printBridgeState();
    if(sem_post(&queueSemaphore)) {
        perror("sem_post");
        exit(EXIT_FAILURE);
    }

    while (cityQueueHead->threadId != threadId) {}
    if(sem_wait(&queueSemaphore)) {
        perror("sem_wait");
        exit(EXIT_FAILURE);
    }
    carOnBridge = threadId;
    carList[threadId - 1].queueId = 0;

    if (info)
        printAllDetails();
    else
        printBridgeState();

    if(sem_post(&queueSemaphore)) {
        perror("sem_post");
        exit(EXIT_FAILURE);
    }

    volatile long long iterations = rand() % 9000 + 1000;
    for (long long i = 0; i < iterations * 100000; i++) { }

    if(sem_wait(&queueSemaphore)) {
        perror("sem_wait");
        exit(EXIT_FAILURE);
    }
    carOnBridge = 0;
    carList[threadId - 1].cityId = cityId == 1 ? 2 : 1;
    queueRemoveFirst(&cityQueueHead);
    if (info)
        printAllDetails();
    else
        printBridgeState();
    if(sem_post(&queueSemaphore)) {
        perror("sem_post");
        exit(EXIT_FAILURE);
    }
    return carList[threadId - 1].cityId;
}
