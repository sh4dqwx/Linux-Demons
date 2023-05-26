#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <getopt.h>
#include "bridge.h"

bool infoFlag = false;
typedef struct args
{
    pthread_t threadId;
    int cityId;
} args;

bool isNumber(char *arg)
{
    for (int i = 0; i < strlen(arg); i++)
    {
        if (!isdigit(arg[i]))
            return false;
    }
    return true;
}

bool validateArguments(int argc, char **argv)
{
    if (argc > 3)
    {
        printf("Niepoprawny format: ./bridgeSimulator <N> [-info]\n");
        return false;
    }
    if (argc == 3 && strcmp(argv[2], "-info"))
    {
        printf("Niepoprawny format: ./bridgeSimulator <N> [-info]\n");
        return false;
    }
    if (!isNumber(argv[1]))
    {
        printf("Podany argument nie jest liczbą\n");
        return false;
    }
    return true;
}

void *car(void *arg)
{
    args tArgs = *(args *)arg;
    pthread_t threadId = tArgs.threadId;
    int cityId = tArgs.cityId;
    while (1)
    {
        // int utime = rand() % 9001 + 1000;
        int utime = rand() % 901 + 100;
        usleep(utime * 1000);
        // printf("Wątek %d wyjeżdża z miasta %s po %d ms.\n", threadId, cityId==1?"A":"B", utime);

        cityId = leaveCity(threadId, cityId);
    }
}

int main(int argc, char **argv)
{
    srand(time(NULL));
    if (!validateArguments(argc, argv))
        return 1;

    int n = atoi(argv[1]);
    if (argc == 3)
        infoFlag = true;
    pthread_t *tIds = (pthread_t *)malloc(n * sizeof(pthread_t));
    args *tArgs = (args *)malloc(n * sizeof(int));
    initBridge(n, infoFlag);

    printf("Tworzenie %d wątków...\n", n);

    for (int i = 0; i < n; i++)
    {
        tArgs[i].threadId = i + 1;
        tArgs[i].cityId = rand() % 2 + 1;
        addCarToCity(tArgs[i].threadId, tArgs[i].cityId);
    }

    for (int i = 0; i < n; i++)
    {
        pthread_create(&tIds[i], NULL, car, &tArgs[i]);
    }

    for (int i = 0; i < n; i++)
    {
        pthread_join(tIds[i], NULL);
    }

    free(tIds);
    free(tArgs);
}