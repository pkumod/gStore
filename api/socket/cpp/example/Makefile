CC=g++
#CC=ccache g++

example: CppAPIExample.o
	$(CC) -o example CppAPIExample.o -L../lib -lgstoreconnector

CppAPIExample.o: CppAPIExample.cpp
	$(CC) -c -I../src/ CppAPIExample.cpp -o CppAPIExample.o

clean:
	rm -rf *.o example
