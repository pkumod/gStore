#CC=g++
CC=g++ -std=c++11
#CC=ccache g++

example: CppAPIExample.o
	$(CC) -o example CppAPIExample.o -L../lib -lgstoreconnector
	#$(CC) -o example CppAPIExample.o -L../lib -lgstoreconnector -L/usr/local/lib -lboost_filesystem

CppAPIExample.o: CppAPIExample.cpp
	$(CC) -c -I../src/ CppAPIExample.cpp -o CppAPIExample.o

clean:
	rm -rf *.o example
