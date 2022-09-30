CXX=g++ -std=c++11

lib_dir=../lib/
inc_log=-I../../../../tools/log4cplus/include

all: $(lib_dir)libgstoreconnector.a

$(lib_dir)libgstoreconnector.a: GstoreConnector.o
	ar -crv $(lib_dir)libgstoreconnector.a GstoreConnector.o

GstoreConnector.o: GstoreConnector.cpp GstoreConnector.h
	$(CXX) -c -I../ GstoreConnector.cpp $(inc_log) -o GstoreConnector.o 

clean:
	rm -rf GstoreConnector.o $(lib_dir)libgstoreconnector.a

