bridgeSimulator: main.o bridge.o queue.o
	gcc main.o bridge.o queue.o -o bridgeSimulator -pthread

main.o: main.c
	gcc -c main.c

bridge.o: bridge.c bridge.h
	gcc -c bridge.c

queue.o: queue.c queue.h
	gcc -c queue.c

clear:
	rm *.o bridgeSimulator