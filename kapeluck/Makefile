CC=g++ -std=c++11

all: prodcon

prodcon: tands.o prodcon.cpp
	$(CC) prodcon.cpp tands.o -o prodcon -lstdc++ -lpthread

tands.o: tands.h tands.c
	gcc -c tands.c

clean:
	rm -rf tands.o
	rm -rf prodcon
