output: main.o
	gcc main.o -o output

main.o: main.c
	gcc -c main.c

clear:
	rm *.o output