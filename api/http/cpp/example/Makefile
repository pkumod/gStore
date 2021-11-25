CC=g++ -std=c++11
#CC=ccache g++

src_dir=../src/

all: Benchmark GET-example POST-example Transaction-example

Benchmark: Benchmark.o
	$(CC) -o Benchmark Benchmark.o -I../ -L../lib -lgstoreconnector -lpthread -lcurl

GET-example: GET-example.o
	$(CC) -o GET-example GET-example.o -I../ -L../lib -lgstoreconnector -lcurl

POST-example: POST-example.o
	$(CC) -o POST-example POST-example.o -I../ -L../lib -lgstoreconnector -lcurl

Transaction-example:Transaction-example.o
	$(CC) -o Transaction-example Transaction-example.o -I../ -L../lib -lgstoreconnector -lcurl

Benchmark.o: Benchmark.cpp $(src_dir)GstoreConnector.cpp $(src_dir)GstoreConnector.h
	$(CC) -c -I../src/ Benchmark.cpp -o Benchmark.o

GET-example.o: GET-example.cpp $(src_dir)GstoreConnector.cpp $(src_dir)GstoreConnector.h
	$(CC) -c -I../src/ GET-example.cpp -o GET-example.o

POST-example.o: POST-example.cpp $(src_dir)GstoreConnector.cpp $(src_dir)GstoreConnector.h
	$(CC) -c -I../src/ POST-example.cpp -o POST-example.o

Transaction-example.o: Transaction-example.cpp $(src_dir)GstoreConnector.cpp $(src_dir)GstoreConnector.h
	$(CC) -c -I../src/ Transaction-example.cpp -o Transaction-example.o

clean:
	rm -rf *.o Benchmark GET-example POST-example Transaction-example
