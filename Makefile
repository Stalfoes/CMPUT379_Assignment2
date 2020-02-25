CC=g++ -std=c++11

all: prodcon

prodcon: prodcon.cpp tands.h tands.c
	gcc -c tands.c
	$(CC) prodcon.cpp tands.o -o prodcon -lstdc++ -lpthread

clean:
	rm -rf prodcon
