#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include "bridge.h"
#include "currentTime.h"

pthread_mutex_t mutexBridge = PTHREAD_MUTEX_INITIALIZER;
currentTime cTime = {0, 0};

void *timeThread(void *arg) {
    while(1) {
        cTime.miliseconds += 100;
        if(cTime.miliseconds == 1000) {
            cTime.seconds++;
            cTime.miliseconds = 0;
        }
        usleep(100000);
    }
}

bool isNumber(char *arg) {
    for(int i=0; i<strlen(arg); i++) {
        if(!isdigit(arg[i])) return false;
    }
    return true;
}

bool validateArguments(int argc, char **argv) {
    if(argc != 2) {
        printf("Niepoprawny format: ./bridgeSimulator <N>\n");
        return false;
    }
    if(!isNumber(argv[1])) {
        printf("Podany argument nie jest liczbą\n");
        return false;
    }
    return true;
}

void *thread(void *arg) {
    int threadId = *(int *)arg;
    int cityId = rand() % 2 + 1;
    while(1) {
        int utime = rand() % 9001 + 1000;
        usleep(utime*1000);
        printf("Wątek %d wyjeżdża z miasta po %d ms.\n", threadId, utime);

        cityId = leaveCity(threadId, cityId, cTime);
    }
}

int main(int argc, char **argv) {
    srand(time(NULL));
    if(!validateArguments(argc, argv))
        return 1;

    int n = atoi(argv[1]);
    pthread_t timerId;
    pthread_t *tIds = malloc(n*sizeof(pthread_t));
    int *tArgs = malloc(n*sizeof(int));
    initBridge(n);
    pthread_create(&timerId, NULL, timeThread, NULL);

    for(int i=0; i<n; i++) {
        tArgs[i] = i+1;
        pthread_create(&tIds[i], NULL, thread, &tArgs[i]);
    }

    for(int i=0; i<n; i++) {
        pthread_join(tIds[i], NULL);
    }

    free(tIds);
    free(tArgs);
}