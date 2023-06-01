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
        int utime = rand() % 9000 + 1000;
        usleep(utime * 1000);

        cityId = leaveCity(threadId, cityId);
    }
}

void *trafficLights(void *arg)
{
    while (1)
    {
        arbiter();
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
    pthread_t *tIds = malloc((n + 1) * sizeof(pthread_t));
    args *tArgs = malloc(n * sizeof(args));
    initBridge(n, infoFlag);

    printf("Tworzenie %d wątków...\n", n);

    for (int i = 0; i < n; i++)
    {
        tArgs[i].threadId = i + 1;
        tArgs[i].cityId = rand() % 2 + 1;
        addCarToCity(tArgs[i].threadId, tArgs[i].cityId);
    }

    pthread_create(&tIds[n], NULL, trafficLights, NULL);

    for (int i = 0; i < n; i++)
    {
        pthread_create(&tIds[i], NULL, car, &tArgs[i]);
    }

    pthread_join(tIds[n], NULL);
    for (int i = 0; i < n; i++)
    {
        pthread_join(tIds[i], NULL);
    }

    free(tIds);
    free(tArgs);
}