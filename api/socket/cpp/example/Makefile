CC=g++ -std=c++11

src_dir=../src/
inc_log=-I../../../../tools/log4cplus/include

all: example 

example: CppAPIExample.o
	$(CC) -o example CppAPIExample.o -I../ -L../lib -lclient -lboost_system

CppAPIExample.o: CppAPIExample.cpp $(src_dir)/Client.cpp $(src_dir)/Client.h
	$(CC) -c -I../src/ CppAPIExample.cpp ${inc_log} -o CppAPIExample.o

clean:
	rm -rf *.o example
