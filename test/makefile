#help for make
#http://www.cnblogs.com/wang_yb/p/3990952.html
#https://segmentfault.com/a/1190000000349917
#http://blog.csdn.net/cuiyifang/article/details/7910268


#compile parameters

CC = g++

CFLAGS = -c -g -Wall

# paths

VPATH = .:Util:Main:Parser:Query:KVstore:Database:Signature:VSTree:Server

SRC_PATH = $(foreach dir, $(subst:,,$(VPATH)), $(wildcard $(dir)/,*.cpp))

OBJ_DIR = .objs/

#OBJ_PATH = $(addprefix $(OBJ_DIR)/, $(subst:,,$(VPATH)))
#MAKE_OBJECT_DIR := $(shell mkdir -p $(OBJ_DIR $(OBJ_PATH)))

OBJS = $(addprefix $(OBJ_DIR),$(subst .cpp,.o,$(SRC_PATH)))

EXE_DIR = bin/

ANTLR = lib/libantlr.a

INC = -I./tools/libantlr3c-3.4/ -I./tools/libantlr3c-3.4/INClude 

#add -lreadline -ltermcap if using readline or objs contain readline
LIBRARY = -ltermcap -lreadline -L./lib -lantlr 

all: $(ANTLR) $(EXE_DIR)gload $(EXE_DIR)gserver $(EXE_DIR)gclient $(EXE_DIR)gquery $(EXE_DIR)gconsole API

#executables begin

$(EXE_DIR)gload: $(OBJS) $(ANTLR)
	$(CC) -g -o $@ $(OBJS) $(LIBRARY)

$(EXE_DIR)gquery: $(OBJS) $(ANTLR)
	$(CC) -g -o $@ $(OBJS) $(LIBRARY)

$(EXE_DIR)gserver: $(OBJS) $(ANTLR)
	$(CC) -g -o $@ $(OBJS) $(LIBRARY)

$(EXE_DIR)gclient: $(OBJS) $(ANTLR)
	$(CC) -g -o $@ $(OBJS) $(LIBRARY)

$(EXE_DIR)gconsole: $(OBJS) $(ANTLR) API
	$(CC) -g -o $@ $(OBJS) $(LIBRARY) -L./api/cpp/lib -lgstoreconnector

#executables end


$(OBJ_DIR)%.o: %.cpp %.cpp.d
	$(CC) -o $@ $< $(INC) $(CFLAGS)


DEPS = $(OBJS:.o=cpp.d)
$(DEPS): $(OBJ_DIR)%.cpp.d: %.cpp
	$(CC) $< -MM $(INC) > $(OBJ_DIR)$<.d


$(ANTLR):
	rm -rf tools/libantlr3c-3.4/
	cd tools; tar -xzvf libantlr3c-3.4.tar.gz;
	cd tools; cd libantlr3c-3.4/; ./configure -enable-64bit; make;
	rm -rf lib/libantlr.a
	ar -crv lib/libantlr.a tools/libantlr3c-3.4/*.o 
	#NOTICE: update the sparql.tar.gz if Sparql* in Parser are changed manually
	rm -rf Parser/Sparql*
	cd tools; tar -xzvf sparql.tar.gz; mv Sparql* ../Parser/;

API: 
	$(MAKE) -C api/cpp/src 
	#$(MAKE) -C api/cpp/example
	$(MAKE) -C api/java/src
	#$(MAKE) -C api/java/example

.PHONY: clean dist tarball api_example gtest sumlines

clean:
	$(MAKE) -C api/cpp/src clean
	$(MAKE) -C api/cpp/example clean
	$(MAKE) -C api/java/src clean
	$(MAKE) -C api/java/example clean
	#$(MAKE) -C KVstore clean
	rm -rf $(EXE_DIR)g* $(OBJ_DIR)*.o
	#rm -rf .project .cproject .settings   just for eclipse
	#rm -rf cscope* just for vim

dist: clean
	rm -rf *.nt *.n3 .debug/*.log .tmp/*.dat *.txt *.db
	rm -rf tools/libantlr3c-3.4 lib/libantlr.a Parser/Sparql*

tarball:
	tar -czvf devGstore.tar.gz api bin lib tools .debug .tmp .objs test docs data makefile \
		Main Database KVstore Util Query Signature VSTree Parser Server LICENSE README.md

api_example: API
	$(MAKE) -C api/cpp/example
	$(MAKE) -C api/java/example

gtest: $(OBJ_DIR)gtest.o $(OBJS)
	$(CC) -g -o $(EXE_DIR)gtest $(OBJ_DIR)gtest.o $(OBJS) lib/libantlr.a $(LIBRARY)

$(OBJ_DIR)gtest.o: test/gtest.cpp
	$(CC) $(CFLAGS) test/gtest.cpp $(INC) -o $(OBJ_DIR)gtest.o
	
sumlines:
	bash test/sumline.sh

