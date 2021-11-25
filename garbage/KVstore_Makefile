CC = g++
CFLAGS = -Wall -Werror -c -g

objdir = ../objs/
obj = $(objdir)Tree.o $(objdir)Storage.o $(objdir)Node.o $(objdir)IntlNode.o \
	  $(objdir)LeafNode.o $(objdir)TBstr.o $(objdir)Util.o $(objdir)Heap.o $(objdir)Hash.o $(objdir)RangeValue.o
all: $(obj)
$(objdir)Tree.o: tree/Tree.cpp
	$(CC) $(CFLAGS) tree/Tree.cpp -o $(objdir)Tree.o
$(objdir)Storage.o: storage/Storage.cpp
	$(CC) $(CFLAGS) storage/Storage.cpp -o $(objdir)Storage.o -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE
$(objdir)Node.o: node/Node.cpp 
	$(CC) $(CFLAGS) node/Node.cpp -o $(objdir)Node.o
$(objdir)IntlNode.o: node/IntlNode.cpp 
	$(CC) $(CFLAGS) node/IntlNode.cpp -o $(objdir)IntlNode.o
$(objdir)LeafNode.o: node/LeafNode.cpp 
	$(CC) $(CFLAGS) node/LeafNode.cpp -o $(objdir)LeafNode.o
$(objdir)TBstr.o: bstr/TBstr.cpp 
	$(CC) $(CFLAGS) bstr/TBstr.cpp -o $(objdir)TBstr.o
$(objdir)Util.o: util/Util.cpp
	$(CC) $(CFLAGS) util/Util.cpp -o $(objdir)Util.o
$(objdir)Hash.o: hash/Hash.cpp
	$(CC) $(CFLAGS) hash/Hash.cpp -o $(objdir)Hash.o
$(objdir)Heap.o: heap/Heap.cpp 
	$(CC) $(CFLAGS) heap/Heap.cpp -o $(objdir)Heap.o
$(objdir)RangeValue.o: rangevalue/RangeValue.cpp
	$(CC) $(CFLAGS) rangevalue/RangeValue.cpp -o $(objdir)RangeValue.o -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE
.PHONY: clean  
clean:
	#nothing to do now

