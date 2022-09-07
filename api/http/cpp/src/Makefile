CXX=g++ -std=c++11

lib_dir=../lib/
socket_obj_dir=../../../../.objs/
inc_log=-I../../../../tools/log4cplus/include

all: $(lib_dir)libgstoreconnector.a

$(lib_dir)libgstoreconnector.a: GstoreConnector.o $(socket_obj_dir)Socket.o
	ar -crv $(lib_dir)libgstoreconnector.a GstoreConnector.o $(socket_obj_dir)Socket.o

GstoreConnector.o: GstoreConnector.cpp GstoreConnector.h $(socket_obj_dir)Socket.o
	$(CXX) -c -I../ GstoreConnector.cpp $(inc_log) -o GstoreConnector.o 

clean:
	rm -rf GstoreConnector.o $(lib_dir)libgstoreconnector.a

