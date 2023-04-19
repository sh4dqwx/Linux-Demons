minicron: main.o list.o
	gcc main.o list.o -o minicron

main.o: main.c
	gcc -c main.c

list.o: list.c list.h
	gcc -c list.c

clear:
	rm *.o minicron