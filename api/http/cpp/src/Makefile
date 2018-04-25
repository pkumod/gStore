#CC=g++
#CC=ccache g++ -std=c++11
CC=g++ -std=c++11

lib_dir=../lib/

all: $(lib_dir)libgstoreconnector.a

$(lib_dir)libgstoreconnector.a: GstoreConnector.o
	ar -crv $(lib_dir)libgstoreconnector.a GstoreConnector.o

GstoreConnector.o: GstoreConnector.cpp GstoreConnector.h
	$(CC) -c -I../ GstoreConnector.cpp -o GstoreConnector.o 

clean:
	rm -rf GstoreConnector.o $(lib_dir)libgstoreconnector.a
