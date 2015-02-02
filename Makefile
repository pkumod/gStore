objdir=objs/
objfile= $(objdir)Bstr.o $(objdir)Database.o $(objdir)KVstore.o $(objdir)Btree.o \
     $(objdir)CBtreeFunc.o $(objdir)SPARQLquery.o $(objdir)BasicQuery.o $(objdir)ResultSet.o \
     $(objdir)SigEntry.o $(objdir)Signature.o $(objdir)Triple.o  $(objdir)util.o $(objdir)VSTree.o \
	 $(objdir)IDList.o $(objdir)EntryBuffer.o $(objdir)LRUCache.o $(objdir)VNode.o $(objdir)DBparser.o \
	 $(objdir)SparqlParser.o $(objdir)SparqlLexer.o  $(objdir)Operation.o $(objdir)Socket.o \
	 $(objdir)Server.o $(objdir)Client.o \
	 $(objdir)TurtleParser.o $(objdir)RDFParser.o
	 
inc=-I./tools/libantlr3c-3.4/ -I./tools/libantlr3c-3.4/include

all: gload gquery gserver gclient

gload: $(objdir)gload.o $(objfile)
	g++ -o gload $(objdir)gload.o $(objfile) lib/libantlr.a 

gquery: $(objdir)gquery.o $(objfile)
	g++ -o gquery $(objdir)gquery.o $(objfile) lib/libantlr.a 

gserver: $(objdir)gserver.o $(objfile)
	g++ -o gserver $(objdir)gserver.o $(objfile)  lib/libantlr.a 

gclient: $(objdir)gclient.o $(objfile)
	g++ -o gclient $(objdir)gclient.o $(objfile)  lib/libantlr.a 
		
$(objdir)gload.o: main/gload.cpp 
	g++ -c main/gload.cpp $(inc) -L./lib lib/libantlr.a -o $(objdir)gload.o 
	
$(objdir)gquery.o: main/gquery.cpp
	g++ -c main/gquery.cpp $(inc) -o $(objdir)gquery.o

$(objdir)gserver.o: main/gserver.cpp
	g++ -c main/gserver.cpp $(inc) -o $(objdir)gserver.o

$(objdir)gclient.o: main/gclient.cpp
	g++ -c main/gclient.cpp $(inc) -o $(objdir)gclient.o
		
$(objdir)Bstr.o: Bstr/Bstr.cpp Bstr/Bstr.h
	g++ -c  Bstr/Bstr.cpp $(inc) -o $(objdir)Bstr.o
	
$(objdir)Database.o: Database/Database.cpp Database/Database.h $(objdir)IDList.o $(objdir)ResultSet.o $(objdir)SPARQLquery.o \
$(objdir)BasicQuery.o \
 $(objdir)Triple.o $(objdir)SigEntry.o $(objdir)KVstore.o $(objdir)VSTree.o $(objdir)DBparser.o $(objdir)util.o \
 $(objdir)RDFParser.o
	g++ -c Database/Database.cpp $(inc) -o $(objdir)Database.o
	
$(objdir)KVstore.o: KVstore/KVstore.cpp KVstore/KVstore.h $(objdir)Btree.o
	g++ -c KVstore/KVstore.cpp $(inc) -o $(objdir)KVstore.o
	
$(objdir)Btree.o:  KVstore/Btree.cpp KVstore/Btree.h $(objdir)CBtreeFunc.o
	g++ -c KVstore/Btree.cpp -o $(objdir)Btree.o
	
$(objdir)CBtreeFunc.o: KVstore/CBtreeFunc.cpp KVstore/CBtreeH.h
	g++ -c KVstore/CBtreeFunc.cpp -o $(objdir)CBtreeFunc.o -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE

$(objdir)IDList.o: Query/IDList.cpp Query/IDList.h
	g++ -c Query/IDList.cpp $(inc) -o $(objdir)IDList.o
$(objdir)SPARQLquery.o: Query/SPARQLquery.cpp Query/SPARQLquery.h $(objdir)BasicQuery.o
	g++ -c Query/SPARQLquery.cpp $(inc) -o $(objdir)SPARQLquery.o
$(objdir)BasicQuery.o: Query/BasicQuery.cpp Query/BasicQuery.h $(objdir)Signature.o
	g++ -c Query/BasicQuery.cpp $(inc) -o $(objdir)BasicQuery.o
$(objdir)ResultSet.o: Query/ResultSet.cpp Query/ResultSet.h
	g++ -c Query/ResultSet.cpp $(inc) -o $(objdir)ResultSet.o

$(objdir)SigEntry.o: Signature/SigEntry.cpp Signature/SigEntry.h $(objdir)Signature.o
	g++ -c Signature/SigEntry.cpp $(inc) -o $(objdir)SigEntry.o
$(objdir)Signature.o: Signature/Signature.cpp Signature/Signature.h
	g++ -c Signature/Signature.cpp $(inc) -o $(objdir)Signature.o
$(objdir)Triple.o: Triple/Triple.cpp Triple/Triple.h
	g++ -c Triple/Triple.cpp $(inc) -o $(objdir)Triple.o
$(objdir)util.o:  util/util.cpp util/util.h
	g++ -c util/util.cpp $(inc) -o $(objdir)util.o
$(objdir)VSTree.o: VSTree/VSTree.cpp VSTree/VSTree.h $(objdir)EntryBuffer.o $(objdir)LRUCache.o $(objdir)VNode.o
	g++ -c VSTree/VSTree.cpp $(inc) -o $(objdir)VSTree.o
$(objdir)EntryBuffer.o: VSTree/EntryBuffer.cpp VSTree/EntryBuffer.h Signature/SigEntry.h
	g++ -c VSTree/EntryBuffer.cpp $(inc) -o $(objdir)EntryBuffer.o
$(objdir)LRUCache.o: VSTree/LRUCache.cpp  VSTree/LRUCache.h VSTree/VNode.h
	g++ -c VSTree/LRUCache.cpp $(inc) -o $(objdir)LRUCache.o
$(objdir)VNode.o: VSTree/VNode.cpp VSTree/VNode.h
	g++ -c VSTree/VNode.cpp $(inc) -o $(objdir)VNode.o
$(objdir)DBparser.o: Parser/DBparser.cpp Parser/DBparser.h $(objdir)SparqlParser.o $(objdir)SparqlLexer.o $(objdir)Triple.o
	g++ -c Parser/DBparser.cpp $(inc) -o $(objdir)DBparser.o
$(objdir)SparqlParser.o: Parser/SparqlParser.c Parser/SparqlParser.h
	gcc  -c Parser/SparqlParser.c $(inc) -o $(objdir)SparqlParser.o
$(objdir)SparqlLexer.o: Parser/SparqlLexer.c Parser/SparqlLexer.h
	gcc  -c Parser/SparqlLexer.c $(inc) -o $(objdir)SparqlLexer.o

$(objdir)TurtleParser.o: Parser/TurtleParser.cpp Parser/TurtleParser.h Parser/Type.h
	gcc  -c Parser/TurtleParser.cpp $(inc) -o $(objdir)TurtleParser.o
$(objdir)RDFParser.o: Parser/RDFParser.cpp Parser/RDFParser.h $(objdir)TurtleParser.o $(objdir)Triple.o
	gcc  -c Parser/RDFParser.cpp $(inc) -o $(objdir)RDFParser.o

$(objdir)Operation.o: Server/Operation.cpp Server/Operation.h
	g++ -c Server/Operation.cpp $(inc) -o $(objdir)Operation.o
$(objdir)Socket.o: Server/Socket.cpp Server/Socket.h
	g++ -c Server/Socket.cpp $(inc) -o $(objdir)Socket.o
$(objdir)Server.o: Server/Server.cpp Server/Server.h $(objdir)Socket.o $(objdir)Database.o $(objdir)Operation.o
	g++ -c Server/Server.cpp $(inc) -o $(objdir)Server.o
$(objdir)Client.o: Server/Client.cpp Server/Client.h $(objdir)Socket.o
	g++ -c Server/Client.cpp $(inc) -o $(objdir)Client.o

lib_antlr:
	rm -rf tools/libantlr3c-3.4/
	cd tools; tar -zxvf libantlr3c-3.4.tar.gz;
	cd tools; cd libantlr3c-3.4/; ./configure -enable-64bit; make;
	rm -rf lib/libantlr.a
	ar -crv lib/libantlr.a tools/libantlr3c-3.4/*.o 

clean:
	rm -rf gload gquery gserver gclient $(objdir)/*.o 

