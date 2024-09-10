CXX= g++ -std=c++11

lib_dir=../lib/
inc_log=-I../../../../include/
socket_obj_dir=../../../../build/src/Server/CMakeFiles/server.dir/

all: $(lib_dir)libclient.a

$(lib_dir)libclient.a: Client.o $(socket_obj_dir)Socket.cpp.o
	ar -crv $(lib_dir)libclient.a Client.o $(socket_obj_dir)Socket.cpp.o

Client.o: Client.cpp Client.h $(socket_obj_dir)Socket.cpp.o
	$(CXX) -c Client.cpp $(inc_log) -o Client.o

clean:
	rm -rf Client.o $(lib_dir)libclient.a
