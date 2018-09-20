CC=g++ -std=c++11
#CC=ccache g++
all: Benchmark CppAPIExample
#all: example Benchmark CppAPIExample

Benchmark: Benchmark.o
	$(CC) -o Benchmark.exe Benchmark.o -lcurl -L../lib -lclient -lpthread

CppAPIExample: CppAPIExample.o
	$(CC) -o CppAPIExample.exe CppAPIExample.o -I../ -lcurl  -L../lib -lgstoreconnector -lclient

Benchmark.o: Benchmark.cpp
	$(CC) -c -I../ Benchmark.cpp -o Benchmark.o

#example: example.o
	#$(CC) -o example example.o -I../ -lcurl -L../lib -lclient

#example.o: example.cpp
	#$(CC) -c -I../ example.cpp -o example.o

CppAPIExample.o: CppAPIExample.cpp
	$(CC) -c -I../ -I../src CppAPIExample.cpp -o CppAPIExample.o
clean:
	rm -rf *.o *.exe
