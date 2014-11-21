example: CppAPIExample.o
	g++ -o example CppAPIExample.o -L../lib -lgstoreconnector

CppAPIExample.o: CppAPIExample.cpp
	g++ -c -I../src/ CppAPIExample.cpp -o CppAPIExample.o

clean:
	rm -rf *.o example
