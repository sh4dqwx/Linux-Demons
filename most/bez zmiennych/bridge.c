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
    if (sem_init(&queueSemaphore, 0, 1) != 0)
    {
        perror("sem_init error");
        exit(EXIT_FAILURE);
    }
    carList = malloc(sizeof(carListElement) * maxCars);
    info = infoFlag;
    printf("Info: %d\n", info);
}

void addCarToCity(pthread_t threadId, int cityId)
{
    carList[carListCount].threadId = threadId;
    carList[carListCount].cityId = cityId;
    carList[carListCount].queueId = 0;
    carListCount++;

    printf("Id wątku: %ld\n", carList[carListCount - 1].threadId);
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
    if (carList[carOnBridge - 1].cityId == -1)
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

    printf("Samochod na moście:\n  - %ld\n", carOnBridge);

    printf("Miasto B:\n- samochody w miescie (%d):\n", carsInCityB);
    for (int i = 0; i < carListCount; i++)
    {
        if (carList[i].cityId != 2)
            continue;
        printf("  - %ld\n", carList[i].threadId);
    }

    printf("- samochody w kolejce (%d):\n", carsInQueueB);
    for (int i = 0; i < carListCount; i++)
    {
        if (carList[i].queueId != 2)
            continue;
        printf("  - %ld\n", carList[i].threadId);
    }
}

int leaveCity(pthread_t threadId, int cityId)
{
    sem_wait(&queueSemaphore);
    queueAdd(&cityQueueHead, threadId, cityId);
    carList[threadId - 1].cityId = 0 - cityId;
    carList[threadId - 1].queueId = cityId;
    if (info)
        printAllDetails();
    sem_post(&queueSemaphore);

    while (cityQueueHead->threadId != threadId)
    {
    }
    sem_wait(&queueSemaphore);
    carOnBridge = threadId;
    carList[threadId - 1].queueId = 0;

    if (info)
        printAllDetails();
    else
        printBridgeState();

    sem_post(&queueSemaphore);

    sleep(1); // Czas przejazdu przez most

    sem_wait(&queueSemaphore);
    carOnBridge = 0;
    carList[threadId - 1].cityId = cityId == -1 ? 2 : 1;
    queueRemoveFirst(&cityQueueHead);
    if (info)
        printAllDetails();
    sem_post(&queueSemaphore);
    return carList[threadId - 1].cityId;
}
