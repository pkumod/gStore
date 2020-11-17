CC=g++ -std=c++11
#CC=ccache g++
all: POST-example GET-example Benchmark Transaction-example

Benchmark: Benchmark.o
	$(CC) -o Benchmark Benchmark.o -L../lib -lclient -lpthread -lcurl

GET-example: GET-example.o
	$(CC) -o GET-example GET-example.o -I../ -L../lib -lclient -lcurl

POST-example: POST-example.o
	$(CC) -o POST-example POST-example.o -I../ -L../lib -lclient -lcurl

Transaction-example:Transaction-example.o
	$(CC) -o Transaction-example Transaction-example.o -I../ -L../lib -lclient -lcurl

GET-example.o: GET-example.cpp ../client.cpp ../client.h
	$(CC) -c -I../ GET-example.cpp -o GET-example.o

Benchmark.o: Benchmark.cpp ../client.cpp ../client.h
	$(CC) -c -I../ Benchmark.cpp -o Benchmark.o
POST-example.o: POST-example.cpp ../client.cpp ../client.h
	$(CC) -c -I../ POST-example.cpp -o POST-example.o

Transaction-example.o: Transaction-example.cpp ../client.cpp ../client.h
	$(CC) -c -I../ Transaction-example.cpp -o Transaction-example.o

clean:
	rm -rf *.o POST-example GET-example Benchmark Transaction-example
