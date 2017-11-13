CC=g++ -std=c++11
#CC=ccache g++

example: example.o
	$(CC) -o example example.o -lcurl -L../lib -lclient

example.o: example.cpp
	$(CC) -c -I../ example.cpp -o example.o

clean:
	rm -rf *.o example
