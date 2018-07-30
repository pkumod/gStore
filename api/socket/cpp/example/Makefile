#CC=g++
CC=g++ -std=c++11
#CC=ccache g++
all: example QueryGenerator test

example: CppAPIExample.o
	$(CC) -o example CppAPIExample.o -L../lib -lgstoreconnector
	#$(CC) -o example CppAPIExample.o -L../lib -lgstoreconnector -L/usr/local/lib -lboost_filesystem

CppAPIExample.o: CppAPIExample.cpp
	$(CC) -c -I../src/ CppAPIExample.cpp -o CppAPIExample.o

QueryGenerator: QueryGenerator.cpp
	$(CC) -o QueryGenerator QueryGenerator.cpp

test: test.o
	$(CC) -o test test.o -L../lib -lgstoreconnector -pthread

test.o: test.cpp
	$(CC) -c -I../src/ test.cpp -o test.o -pthread

clean:
	rm -rf *.o example QueryGenerator test
