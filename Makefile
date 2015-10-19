CC = g++
CFLAGS = -c -g -Wall

objdir=objs/

btreeobj = $(objdir)Tree.o $(objdir)Storage.o $(objdir)Node.o $(objdir)IntlNode.o $(objdir)LeafNode.o $(objdir)TBstr.o $(objdir)Util.o $(objdir)Heap.o $(objdir)Hash.o $(objdir)RangeValue.o 

objfile=$(objdir)Bstr.o $(objdir)Database.o $(objdir)KVstore.o $(objdir)SPARQLquery.o $(objdir)BasicQuery.o \
		$(objdir)ResultSet.o $(objdir)SigEntry.o $(objdir)Signature.o $(objdir)Triple.o  $(objdir)util.o \
		$(objdir)VSTree.o $(objdir)IDList.o $(objdir)EntryBuffer.o $(objdir)LRUCache.o $(objdir)VNode.o \
		$(objdir)DBparser.o $(objdir)SparqlParser.o $(objdir)SparqlLexer.o  $(objdir)Operation.o \
		$(objdir)Socket.o $(objdir)Server.o $(objdir)Client.o $(objdir)TurtleParser.o $(objdir)RDFParser.o \
		$(btreeobj)
	 
inc=-I./tools/libantlr3c-3.4/ -I./tools/libantlr3c-3.4/include

all: lib_antlr btree gload gquery gserver gclient gtest
btree:
	$(MAKE) -C KVstore

gload: $(objdir)gload.o $(objfile)
	$(CC) -g -o gload $(objdir)gload.o $(objfile) lib/libantlr.a 

gquery: $(objdir)gquery.o $(objfile)
	$(CC) -lreadline -ltermcap -g -o gquery $(objdir)gquery.o $(objfile) lib/libantlr.a 

gserver: $(objdir)gserver.o $(objfile)
	$(CC) -g -o gserver $(objdir)gserver.o $(objfile)  lib/libantlr.a 

gclient: $(objdir)gclient.o $(objfile)
	$(CC) -g -o gclient $(objdir)gclient.o $(objfile)  lib/libantlr.a 
gtest: $(objdir)gtest.o $(objfile)
	$(CC) -g -o gtest $(objdir)gtest.o $(objfile) lib/libantlr.a
		
$(objdir)gload.o: main/gload.cpp 
	$(CC) $(CFLAGS) main/gload.cpp $(inc) -L./lib lib/libantlr.a -o $(objdir)gload.o 
	
$(objdir)gquery.o: main/gquery.cpp
	$(CC) $(CFLAGS) main/gquery.cpp $(inc) -o $(objdir)gquery.o

$(objdir)gserver.o: main/gserver.cpp
	$(CC) $(CFLAGS) main/gserver.cpp $(inc) -o $(objdir)gserver.o

$(objdir)gclient.o: main/gclient.cpp
	$(CC) $(CFLAGS) main/gclient.cpp $(inc) -o $(objdir)gclient.o
$(objdir)gtest.o: main/gtest.cpp
	$(CC) $(CFLAGS) main/gtest.cpp $(inc) -o $(objdir)gtest.o
		
$(objdir)Bstr.o: Bstr/Bstr.cpp Bstr/Bstr.h
	$(CC) $(CFLAGS)  Bstr/Bstr.cpp $(inc) -o $(objdir)Bstr.o
	
$(objdir)Database.o: Database/Database.cpp Database/Database.h $(objdir)IDList.o $(objdir)ResultSet.o $(objdir)SPARQLquery.o \
$(objdir)BasicQuery.o \
 $(objdir)Triple.o $(objdir)SigEntry.o $(objdir)KVstore.o $(objdir)VSTree.o $(objdir)DBparser.o $(objdir)util.o \
 $(objdir)RDFParser.o
	$(CC) $(CFLAGS) Database/Database.cpp $(inc) -o $(objdir)Database.o
	
$(objdir)KVstore.o: KVstore/KVstore.cpp KVstore/KVstore.h 
	$(CC) $(CFLAGS) KVstore/KVstore.cpp $(inc) -o $(objdir)KVstore.o

$(objdir)IDList.o: Query/IDList.cpp Query/IDList.h
	$(CC) $(CFLAGS) Query/IDList.cpp $(inc) -o $(objdir)IDList.o
$(objdir)SPARQLquery.o: Query/SPARQLquery.cpp Query/SPARQLquery.h $(objdir)BasicQuery.o
	$(CC) $(CFLAGS) Query/SPARQLquery.cpp $(inc) -o $(objdir)SPARQLquery.o
$(objdir)BasicQuery.o: Query/BasicQuery.cpp Query/BasicQuery.h $(objdir)Signature.o
	$(CC) $(CFLAGS) Query/BasicQuery.cpp $(inc) -o $(objdir)BasicQuery.o
$(objdir)ResultSet.o: Query/ResultSet.cpp Query/ResultSet.h
	$(CC) $(CFLAGS) Query/ResultSet.cpp $(inc) -o $(objdir)ResultSet.o

$(objdir)SigEntry.o: Signature/SigEntry.cpp Signature/SigEntry.h $(objdir)Signature.o
	$(CC) $(CFLAGS) Signature/SigEntry.cpp $(inc) -o $(objdir)SigEntry.o
$(objdir)Signature.o: Signature/Signature.cpp Signature/Signature.h
	$(CC) $(CFLAGS) Signature/Signature.cpp $(inc) -o $(objdir)Signature.o
$(objdir)Triple.o: Triple/Triple.cpp Triple/Triple.h
	$(CC) $(CFLAGS) Triple/Triple.cpp $(inc) -o $(objdir)Triple.o
$(objdir)util.o:  util/util.cpp util/util.h
	$(CC) $(CFLAGS) util/util.cpp $(inc) -o $(objdir)util.o
$(objdir)VSTree.o: VSTree/VSTree.cpp VSTree/VSTree.h $(objdir)EntryBuffer.o $(objdir)LRUCache.o $(objdir)VNode.o
	$(CC) $(CFLAGS) VSTree/VSTree.cpp $(inc) -o $(objdir)VSTree.o
$(objdir)EntryBuffer.o: VSTree/EntryBuffer.cpp VSTree/EntryBuffer.h Signature/SigEntry.h
	$(CC) $(CFLAGS) VSTree/EntryBuffer.cpp $(inc) -o $(objdir)EntryBuffer.o
$(objdir)LRUCache.o: VSTree/LRUCache.cpp  VSTree/LRUCache.h VSTree/VNode.h
	$(CC) $(CFLAGS) VSTree/LRUCache.cpp $(inc) -o $(objdir)LRUCache.o
$(objdir)VNode.o: VSTree/VNode.cpp VSTree/VNode.h
	$(CC) $(CFLAGS) VSTree/VNode.cpp $(inc) -o $(objdir)VNode.o
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

$(objdir)Operation.o: Server/Operation.cpp Server/Operation.h
	$(CC) $(CFLAGS) Server/Operation.cpp $(inc) -o $(objdir)Operation.o
$(objdir)Socket.o: Server/Socket.cpp Server/Socket.h
	$(CC) $(CFLAGS) Server/Socket.cpp $(inc) -o $(objdir)Socket.o
$(objdir)Server.o: Server/Server.cpp Server/Server.h $(objdir)Socket.o $(objdir)Database.o $(objdir)Operation.o
	$(CC) $(CFLAGS) Server/Server.cpp $(inc) -o $(objdir)Server.o
$(objdir)Client.o: Server/Client.cpp Server/Client.h $(objdir)Socket.o
	$(CC) $(CFLAGS) Server/Client.cpp $(inc) -o $(objdir)Client.o

lib_antlr:
	rm -rf tools/libantlr3c-3.4/
	cd tools; tar -zxvf libantlr3c-3.4.tar.gz;
	cd tools; cd libantlr3c-3.4/; ./configure -enable-64bit; make;
	rm -rf lib/libantlr.a
	ar -crv lib/libantlr.a tools/libantlr3c-3.4/*.o 

clean:
	$(MAKE) -C KVstore clean
	rm -rf gload gquery gtest gserver gclient $(objdir)/*.o lib/libantlr.a
	rm -rf *.log *.nt tools/libantlr3c-3.4

