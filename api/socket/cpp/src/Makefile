CXX= g++ -std=c++11

lib_dir=../lib/
socket_obj_dir=../../../../.objs/

all: $(lib_dir)libclient.a

$(lib_dir)libclient.a: Client.o $(socket_obj_dir)Socket.o
	ar -crv $(lib_dir)libclient.a Client.o $(socket_obj_dir)Socket.o

Client.o: Client.cpp Client.h $(socket_obj_dir)Socket.o
	$(CXX) -c Client.cpp -o Client.o

clean:
	rm -rf Client.o $(lib_dir)libclient.a
