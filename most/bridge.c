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

void initBridge(int maxCars, bool infoFlag) {
    if (sem_init(&queueSemaphore, 0, 1) != 0) {
        perror("sem_init error");
        exit(EXIT_FAILURE);
    }
    carList = malloc(sizeof(carListElement) * maxCars);
    info = infoFlag;
}

void addCarToCity(pthread_t threadId, int cityId) {
    sem_wait(&queueSemaphore);
    carList[carListCount].threadId = threadId;
    carList[carListCount].cityId = cityId;
    carList[carListCount].queueId = 0;
    carListCount++;
    sem_post(&queueSemaphore);
}

int countCarsInCity(carListElement *carList_copy, int cityId) {
    int count = 0;
    for (int i = 0; i < carListCount; i++) {
        if(carList_copy[i].cityId != cityId) continue;
        count++;
    }
    return count;
}

int countCarsInQueue(carListElement *carList_copy, int queueId) {
    int count = 0;
    for (int i = 0; i < carListCount; i++) {
        if(carList_copy[i].queueId != queueId) continue;
        count++;
    }
    return count;
}

void printBridgeState() {
    carListElement *carList_freezed = malloc(sizeof(carListElement) * carListCount);
    for (int i = 0; i < carListCount; i++) {
        carList_freezed[i].threadId = carList[i].threadId;
        carList_freezed[i].cityId = carList[i].cityId;
        carList_freezed[i].queueId = carList[i].queueId;
    }
    pthread_t freezed_carOnBridge = carOnBridge;

    if(carList_freezed[carOnBridge - 1].cityId == 1) 
        printf("A-%d %d --> [>> %ld >>] <-- %d %d-B\n",
        countCarsInCity(carList_freezed, 1),
        countCarsInQueue(carList_freezed, 1),
        freezed_carOnBridge,
        countCarsInQueue(carList_freezed, 2),
        countCarsInCity(carList_freezed, 2));
    else 
        printf("A-%d %d --> [<< %ld <<] <-- %d %d-B\n",
        countCarsInCity(carList_freezed, 1),
        countCarsInQueue(carList_freezed, 1),
        freezed_carOnBridge,
        countCarsInQueue(carList_freezed, 2),
        countCarsInCity(carList_freezed, 2));
}

void printAllDetails() {
    carListElement *carList_freezed = malloc(sizeof(carListElement) * carListCount);
    for (int i = 0; i < carListCount; i++) {
        carList_freezed[i].threadId = carList[i].threadId;
        carList_freezed[i].cityId = carList[i].cityId;
        carList_freezed[i].queueId = carList[i].queueId;
    }

    int freezed_carsInCityA = countCarsInCity(carList_freezed, 1);
    int freezed_carsInCityB = countCarsInCity(carList_freezed, 2);
    int freezed_carsInQueueA = countCarsInQueue(carList_freezed, 1);
    int freezed_carsInQueueB = countCarsInQueue(carList_freezed, 2);
    pthread_t freezed_carOnBridge = carOnBridge;

    printf("-----------------------------------------------------\n");
    printf("Miasto A:\n- samochody w miescie (%d):\n", freezed_carsInCityA);
    for (int i = 0; i < carListCount; i++)
    {
        if(carList[i].cityId != 1) continue;
        printf("  - %ld\n", carList[i].threadId);
    }
    
    printf("- samochody w kolejce A (%d):\n", freezed_carsInQueueA);
    for (int i = 0; i < carListCount; i++)
    {
        if(carList[i].queueId != 1) continue;
        printf("  - %ld\n", carList[i].threadId);
    }

    printf("Samochod na moście:\n  - %ld\n", freezed_carOnBridge);

    printf("Miasto B:\n- samochody w miescie (%d):\n", freezed_carsInCityB);
    for (int i = 0; i < carListCount; i++)
    {
        if(carList[i].cityId != 2) continue;
        printf("  - %ld\n", carList[i].threadId);
    }

    printf("- samochody w kolejce B (%d):\n", freezed_carsInQueueB);
    for (int i = 0; i < carListCount; i++)
    {
        if(carList[i].queueId != 2) continue;
        printf("  - %ld\n", carList[i].threadId);
    }
}

int leaveCity(pthread_t threadId, int cityId) {
    sem_wait(&queueSemaphore);
    queueAdd(&cityQueueHead, threadId, cityId);
    carList[threadId - 1].cityId = 0;
    carList[threadId - 1].queueId = cityId;
    if(info) printAllDetails();
    sem_post(&queueSemaphore);

    while (cityQueueHead->threadId != threadId) {}

    carOnBridge = threadId;
    carList[threadId - 1].queueId = 0;
    if(info) printAllDetails();
    else printBridgeState();

    sleep(1); //Czas przejazdu przez most

    sem_wait(&queueSemaphore);
    carOnBridge = 0;
    carList[threadId - 1].cityId = cityId == 1 ? 2 : 1;
    queueRemoveFirst(&cityQueueHead);
    if(info) printAllDetails();
    sem_post(&queueSemaphore);
    return carList[threadId - 1].cityId;
}
