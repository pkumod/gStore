CC = g++
CFLAGS = -c -g -Wall

objdir = .objs/

kvstoreobj = $(objdir)KVstore.o $(objdir)Tree.o $(objdir)Storage.o $(objdir)Node.o \
		$(objdir)IntlNode.o $(objdir)LeafNode.o $(objdir)Heap.o 
	#$(objdir)RangeValue.o #$(objdir)Util.o $(objdir)TBstr.o $(objdir)Hash.o 
utilobj = $(objdir)Util.o $(objdir)Stream.o
queryobj = $(objdir)SPARQLquery.o $(objdir)BasicQuery.o $(objdir)ResultSet.o \
		$(objdir)IDList.o 
signatureobj = $(objdir)SigEntry.o $(objdir)Signature.o
vstreeobj = $(objdir)VSTree.o $(objdir)EntryBuffer.o $(objdir)LRUCache.o \
		$(objdir)VNode.o
parserobj = $(objdir)RDFParser.o $(objdir)DBparser.o $(objdir)SparqlParser.o \
			$(objdir)SparqlLexer.o $(objdir)TurtleParser.o
serverobj = $(objdir)Operation.o $(objdir)Server.o $(objdir)Client.o \
			$(objdir)Socket.o 
bstrobj = $(objdir)Bstr.o
databaseobj = $(objdir)Database.o
tripleobj = $(objdir)Triple.o

objfile = $(kvstoreobj) $(vstreeobj) $(parserobj) $(serverobj) \
		  $(bstrobj) $(databaseobj) $(tripleobj) $(utilobj) \
		  $(signatureobj) $(queryobj)
	 
inc = -I./tools/libantlr3c-3.4/ -I./tools/libantlr3c-3.4/include

#gquery 
all: lib_antlr gload gserver gclient gtest gquery

#init: lib_antlr kvstore vstree parser server bstr database triple \
#		util signature query
#kvstore:
#	#$(MAKE) -C KVstore
#	$(CC) $(CFLAGS) KVstore/tree/Tree.cpp -o $(objdir)Tree.o
#	$(CC) $(CFLAGS) KVstore/storage/Storage.cpp -o $(objdir)Storage.o -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE
#	$(CC) $(CFLAGS) KVstore/node/Node.cpp -o $(objdir)Node.o
#	$(CC) $(CFLAGS) KVstore/node/IntlNode.cpp -o $(objdir)IntlNode.o
#	$(CC) $(CFLAGS) KVstore/node/LeafNode.cpp -o $(objdir)LeafNode.o
#	#$(CC) $(CFLAGS) KVstore/bstr/TBstr.cpp -o $(objdir)TBstr.o
#	#$(CC) $(CFLAGS) KVstore/hash/Hash.cpp -o $(objdir)Hash.o
#	$(CC) $(CFLAGS) KVstore/heap/Heap.cpp -o $(objdir)Heap.o
#	#$(CC) $(CFLAGS) KVstore/rangevalue/RangeValue.cpp -o $(objdir)RangeValue.o -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE
#	$(CC) $(CFLAGS) KVstore/KVstore.cpp -o $(objdir)KVstore.o


#executables begin

gload: $(objdir)gload.o $(objfile)
	$(CC) -g -o gload $(objdir)gload.o $(objfile) lib/libantlr.a 

gquery: $(objdir)gquery.o $(objfile)
	$(CC) -ltermcap -lreadline -g -o gquery $(objdir)gquery.o $(objfile) lib/libantlr.a 
	#add -lreadline -ltermcap if using readline

gserver: $(objdir)gserver.o $(objfile)
	$(CC) -g -o gserver $(objdir)gserver.o $(objfile)  lib/libantlr.a 

gclient: $(objdir)gclient.o $(objfile)
	$(CC) -g -o gclient $(objdir)gclient.o $(objfile)  lib/libantlr.a 
gtest: $(objdir)gtest.o $(objfile)
	$(CC) -g -o gtest $(objdir)gtest.o $(objfile) lib/libantlr.a

#executables end


#objects in Main/ begin

$(objdir)gload.o: Main/gload.cpp 
	$(CC) $(CFLAGS) Main/gload.cpp $(inc) -L./lib lib/libantlr.a -o $(objdir)gload.o 
	
$(objdir)gquery.o: Main/gquery.cpp
	$(CC) $(CFLAGS) Main/gquery.cpp $(inc) -o $(objdir)gquery.o -DREADLINE_ON
	#add -DREADLINE_ON if using readline

$(objdir)gserver.o: Main/gserver.cpp
	$(CC) $(CFLAGS) Main/gserver.cpp $(inc) -o $(objdir)gserver.o

$(objdir)gclient.o: Main/gclient.cpp
	$(CC) $(CFLAGS) Main/gclient.cpp $(inc) -o $(objdir)gclient.o

$(objdir)gtest.o: Main/gtest.cpp
	$(CC) $(CFLAGS) Main/gtest.cpp $(inc) -o $(objdir)gtest.o
	
#objects in Main/ end


#objects in kvstore/ begin

$(objdir)Tree.o: KVstore/tree/Tree.cpp KVstore/tree/Tree.h
	$(CC) $(CFLAGS) KVstore/tree/Tree.cpp -o $(objdir)Tree.o

$(objdir)Storage.o: KVstore/storage/Storage.cpp KVstore/storage/Storage.h
	$(CC) $(CFLAGS) KVstore/storage/Storage.cpp -o $(objdir)Storage.o -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE

$(objdir)Node.o: KVstore/node/Node.cpp KVstore/node/Node.h
	$(CC) $(CFLAGS) KVstore/node/Node.cpp -o $(objdir)Node.o

$(objdir)IntlNode.o: KVstore/node/IntlNode.cpp KVstore/node/IntlNode.h
	$(CC) $(CFLAGS) KVstore/node/IntlNode.cpp -o $(objdir)IntlNode.o

$(objdir)LeafNode.o: KVstore/node/LeafNode.cpp KVstore/node/LeafNode.h
	$(CC) $(CFLAGS) KVstore/node/LeafNode.cpp -o $(objdir)LeafNode.o

$(objdir)Heap.o: KVstore/heap/Heap.cpp KVstore/heap/Heap.h
	$(CC) $(CFLAGS) KVstore/heap/Heap.cpp -o $(objdir)Heap.o

#$(objdir)RangeValue.o: KVstore/rangevalue/RangeValue.cpp KVstore/rangevalue/RangeValue.h
#	$(CC) $(CFLAGS) KVstore/rangevalue/RangeValue.cpp -o $(objdir)RangeValue.o

$(objdir)KVstore.o: KVstore/KVstore.cpp KVstore/KVstore.h
	$(CC) $(CFLAGS) KVstore/KVstore.cpp $(inc) -o $(objdir)KVstore.o

#objects in kvstore/ end


#objects in Bstr/ begin

$(objdir)Bstr.o: Bstr/Bstr.cpp Bstr/Bstr.h
	$(CC) $(CFLAGS)  Bstr/Bstr.cpp -o $(objdir)Bstr.o
	
#objects in bstr/ end


#objects in Database/ begin

$(objdir)Database.o: Database/Database.cpp Database/Database.h \
	$(objdir)IDList.o $(objdir)ResultSet.o $(objdir)SPARQLquery.o \
	$(objdir)BasicQuery.o $(objdir)Triple.o $(objdir)SigEntry.o \
	$(objdir)KVstore.o $(objdir)VSTree.o $(objdir)DBparser.o \
	$(objdir)Util.o $(objdir)RDFParser.o
	$(CC) $(CFLAGS) Database/Database.cpp $(inc) -o $(objdir)Database.o

#objects in Database/ end


#objects in Query/ begin

$(objdir)IDList.o: Query/IDList.cpp Query/IDList.h
	$(CC) $(CFLAGS) Query/IDList.cpp $(inc) -o $(objdir)IDList.o

$(objdir)SPARQLquery.o: Query/SPARQLquery.cpp Query/SPARQLquery.h $(objdir)BasicQuery.o
	$(CC) $(CFLAGS) Query/SPARQLquery.cpp $(inc) -o $(objdir)SPARQLquery.o

$(objdir)BasicQuery.o: Query/BasicQuery.cpp Query/BasicQuery.h $(objdir)Signature.o
	$(CC) $(CFLAGS) Query/BasicQuery.cpp $(inc) -o $(objdir)BasicQuery.o
$(objdir)ResultSet.o: Query/ResultSet.cpp Query/ResultSet.h
	$(CC) $(CFLAGS) Query/ResultSet.cpp $(inc) -o $(objdir)ResultSet.o

#objects in Query/ end


#objects in Signature/ begin

$(objdir)SigEntry.o: Signature/SigEntry.cpp Signature/SigEntry.h $(objdir)Signature.o
	$(CC) $(CFLAGS) Signature/SigEntry.cpp $(inc) -o $(objdir)SigEntry.o

$(objdir)Signature.o: Signature/Signature.cpp Signature/Signature.h
	$(CC) $(CFLAGS) Signature/Signature.cpp $(inc) -o $(objdir)Signature.o

#objects in Signature/ end


#objects in Triple begin

$(objdir)Triple.o: Triple/Triple.cpp Triple/Triple.h
	$(CC) $(CFLAGS) Triple/Triple.cpp -o $(objdir)Triple.o

#objects in Triple end


#objects in Util/ begin

$(objdir)Util.o:  Util/Util.cpp Util/Util.h
	$(CC) $(CFLAGS) Util/Util.cpp -o $(objdir)Util.o

$(objdir)Stream.o:  Util/Stream.cpp Util/Stream.h
	$(CC) $(CFLAGS) Util/Stream.cpp -o $(objdir)Stream.o -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURC

#objects in util/ end


#objects in VSTree/ begin

$(objdir)VSTree.o: VSTree/VSTree.cpp VSTree/VSTree.h $(objdir)EntryBuffer.o $(objdir)LRUCache.o $(objdir)VNode.o
	$(CC) $(CFLAGS) VSTree/VSTree.cpp $(inc) -o $(objdir)VSTree.o

$(objdir)EntryBuffer.o: VSTree/EntryBuffer.cpp VSTree/EntryBuffer.h Signature/SigEntry.h
	$(CC) $(CFLAGS) VSTree/EntryBuffer.cpp $(inc) -o $(objdir)EntryBuffer.o

$(objdir)LRUCache.o: VSTree/LRUCache.cpp  VSTree/LRUCache.h VSTree/VNode.h
	$(CC) $(CFLAGS) VSTree/LRUCache.cpp $(inc) -o $(objdir)LRUCache.o

$(objdir)VNode.o: VSTree/VNode.cpp VSTree/VNode.h
	$(CC) $(CFLAGS) VSTree/VNode.cpp $(inc) -o $(objdir)VNode.o

#objects in VSTree/ end


#objects in Parser/ begin

$(objdir)DBparser.o: Parser/DBparser.cpp Parser/DBparser.h $(objdir)SparqlParser.o $(objdir)SparqlLexer.o $(objdir)Triple.o
	$(CC) $(CFLAGS) Parser/DBparser.cpp $(inc) -o $(objdir)DBparser.o

$(objdir)SparqlParser.o: Parser/SparqlParser.c Parser/SparqlParser.h
	gcc  $(CFLAGS) Parser/SparqlParser.c $(inc) -o $(objdir)SparqlParser.o

$(objdir)SparqlLexer.o: Parser/SparqlLexer.c Parser/SparqlLexer.h
	gcc  $(CFLAGS) Parser/SparqlLexer.c $(inc) -o $(objdir)SparqlLexer.o

$(objdir)TurtleParser.o: Parser/TurtleParser.cpp Parser/TurtleParser.h Parser/Type.h
	gcc  $(CFLAGS) Parser/TurtleParser.cpp $(inc) -o $(objdir)TurtleParser.o

$(objdir)RDFParser.o: Parser/RDFParser.cpp Parser/RDFParser.h $(objdir)TurtleParser.o $(objdir)Triple.o
	gcc  $(CFLAGS) Parser/RDFParser.cpp $(inc) -o $(objdir)RDFParser.o

#objects in Parser/ end


#objects in Server/ begin

$(objdir)Operation.o: Server/Operation.cpp Server/Operation.h
	$(CC) $(CFLAGS) Server/Operation.cpp $(inc) -o $(objdir)Operation.o

$(objdir)Socket.o: Server/Socket.cpp Server/Socket.h
	$(CC) $(CFLAGS) Server/Socket.cpp $(inc) -o $(objdir)Socket.o

$(objdir)Server.o: Server/Server.cpp Server/Server.h $(objdir)Socket.o $(objdir)Database.o $(objdir)Operation.o
	$(CC) $(CFLAGS) Server/Server.cpp $(inc) -o $(objdir)Server.o

$(objdir)Client.o: Server/Client.cpp Server/Client.h $(objdir)Socket.o
	$(CC) $(CFLAGS) Server/Client.cpp $(inc) -o $(objdir)Client.o

#objects in Server/ end


lib_antlr:
	rm -rf tools/libantlr3c-3.4/
	cd tools; tar -zxvf libantlr3c-3.4.tar.gz;
	cd tools; cd libantlr3c-3.4/; ./configure -enable-64bit; make;
	rm -rf lib/libantlr.a
	ar -crv lib/libantlr.a tools/libantlr3c-3.4/*.o 

.PHONY: clean dist tarball

clean:
	#$(MAKE) -C KVstore clean
	rm -rf gtest gload gquery gserver gclient $(objdir)*.o 
	rm -rf .project .cproject

dist: clean
	rm -rf *.nt .debug/*.log .tmp/*.dat *.txt
	rm -rf tools/libantlr3c-3.4 lib/libantlr.a 

tarball:
	tar -czvf devGstore.tar.gz Main Bstr lib tools .settings .debug .tmp .objs Server Triple\
		Database KVstore Util Query Signature VSTree Parser makefile example api README.md

