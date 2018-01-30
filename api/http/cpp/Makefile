CC= g++ -std=c++11

lib_dir=lib/

all: $(lib_dir)libclient.a

$(lib_dir)libclient.a: client.o
	ar -crv $(lib_dir)libclient.a client.o

client.o: client.cpp client.h
	$(CC) -c client.cpp -o client.o
clean:
	rm -rf client.o $(lib_dir)libclient.a
