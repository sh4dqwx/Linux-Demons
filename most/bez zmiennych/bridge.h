#ifndef BRIDGE_H
#define BRIDGE_H

void printCarList();
void initBridge(int maxCars, bool infoFlag);
void addCarToCity(pthread_t threadId, int cityId);
void printBridgeState(pthread_t threadId, int cityId);
int leaveCity(pthread_t threadId, int cityId);

#endif