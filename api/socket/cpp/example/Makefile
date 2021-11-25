CC=g++ -std=c++11
all: example 

example: CppAPIExample.o
	$(CC) -o example CppAPIExample.o -L../lib -lclient -lboost_system

CppAPIExample.o: CppAPIExample.cpp
	$(CC) -c -I../src/ CppAPIExample.cpp -o CppAPIExample.o

clean:
	rm -rf *.o example
