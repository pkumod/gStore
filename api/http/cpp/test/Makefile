CC=g++ -std=c++11
test: test.o
	$(CC) -o test test.o
test.o: test.cpp
	$(CC) -c test.cpp -o test.o
clean:
	rm -rf *.o test
