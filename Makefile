minicron: minicron.o list.o
	gcc minicron.o list.o -o minicron

minicron.o: minicron.c
	gcc -c minicron.c

list.o: list.c list.h
	gcc -c list.c

clear:
	rm *.o minicron