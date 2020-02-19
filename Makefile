CC=g++ -std=c++11

all: prodcon

prodcon: prodcon.cpp tands.c
	$(CC) prodcon.cpp tands.c -o prodcon -lstdc++

clean:
	rm -rf prodcon
