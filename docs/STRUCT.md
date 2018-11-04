**This chapter introduce the whole structure of the gStore system project.**

#### Figures

The whole architecture of gStore system is presented in [Architecture](png/system_architecture_en.png).
The thread model of 'ghttp' can be viewed in [EN](png/ghttp_thread_model_en.png) and [ZH](png/ghttp_thread_model_zh.png), which shows the relationship among main process, sever thread, query thread and so on.
The flow of answering a SPARQL query is given in [SPARQL Processing](png/sparql_processing.png), and the subprocess, which only targets at the BGP(Basic Graph Pattern) processing, is drawed in [BGP Processing](png/BGP.png).

---

#### The core source codes are listed below:

- Database/ (calling other core parts to deal with requests from interface part)
	
	- Database.cpp (achieve functions)

	- Database.h (class, members and functions definitions)
	
	- Join.cpp (join the node candidates to get results)

	- Join.h (class, members and functions definitions)

	- Strategy.cpp 

	- Strategy.h

- KVstore/ (a key-value store to swap between memory and disk)
	
	- KVstore.cpp (interact with upper layers)

	- KVstore.h

	- ISArray/

		- ISArray.cpp

		- ISArray.h

		- ISBlockManager.cpp

		- ISBlockManager.h

		- ISEntry.cpp

		- ISEntry.h

	- ISTree/

		- ISTree.cpp

		- ISTree.h

		- heap/ (a heap of nodes whose content are in memory)
		
			- ISHeap.cpp

			- ISHeap.h
	
		- node/ (all kinds of nodes in B+-tree)

			- ISIntlNode.cpp
		
			- ISIntlNode.h

			- ISLeafNode.cpp

			- ISLeafNode.h

			- ISNode.cpp

			- ISNode.h

		- storage/

			- ISStorage.cpp

			- ISStorage.h

	- IVArray/

		- IVArray.cpp

		- IVArray.h

		- IVBlockManager.cpp

		- IVBlockManager.h

		- IVCacheManager.cpp

		- IVCacheManger.h

		- IVEntry.cpp

		- IVEntry.h

	- IVTree/

		- IVTree.cpp

		- IVTree.h

		- heap/ (a heap of nodes whose content are in memory)
		
			- IVHeap.cpp

			- IVHeap.h
	
		- node/ (all kinds of nodes in B+-tree)

			- IVIntlNode.cpp
		
			- IVIntlNode.h

			- IVLeafNode.cpp

			- IVLeafNode.h

			- IVNode.cpp

			- IVNode.h

		- storage/

			- IVStorage.cpp

			- IVStorage.h

	- SITree/

		- SITree.cpp

		- SITree.h

		- heap/ (a heap of nodes whose content are in memory)
		
			- SIHeap.cpp

			- SIHeap.h
	
		- node/ (all kinds of nodes in B+-tree)

			- SIIntlNode.cpp
		
			- SIIntlNode.h

			- SILeafNode.cpp

			- SILeafNode.h

			- SINode.cpp

			- SINode.h

		- storage/

			- SIStorage.cpp

			- SIStorage.h

- Query/ (needed to answer SPARQL query)

	- BasicQuery.cpp (basic type of queries without aggregate operations)

	- BasicQuery.h

	- IDList.cpp (candidate list of a node/variable in query)

	- IDList.h 

	- ResultFilter.cpp

	- ResultFilter.h

	- ResultSet.cpp (keep the result set corresponding to a query)

	- ResultSet.h

	- SPARQLquery.cpp (deal with a entire SPARQL query)

	- SPARQLquery.h

	- Varset.cpp

	- Varset.h

	- QueryCache.cpp

	- QueryCache.h

	- QueryTree.cpp

	- QueryTree.h

	- GeneralEvaluation.cpp

	- GeneralEvaluation.h

	- TempResult.cpp

	- TempResult.h

	- RegexExpression.h

- Signature/ (assign signatures for nodes and edges, but not for literals)

	- SigEntry.cpp

	- SigEntry.h

	- Signature.cpp

	- Signature.h

- VSTree/ (an tree index to prune more efficiently)

	- EntryBuffer.cpp

	- EntryBuffer.h

	- LRUCache.cpp

	- LRUCache.h
	
	- VNode.cpp
	
	- VNode.h
	
	- VSTree.cpp
	
	- VSTree.h

- - -

#### The parser part is listed below:

- Parser/
	
	- DBParser.cpp

	- DBParser.h

	- RDFParser.cpp

	- RDFParser.h

	- SparqlParser.c (auto-generated, subtle modified manually, compressed)

	- SparqlParser.h (auto-generated, subtle modified manually, compressed)

	- SparqlLexer.c (auto-generated, subtle modified manually, compressed)

	- SparqlLexer.h (auto-generated, subtle modified manually, compressed)

	- TurtleParser.cpp

	- TurtleParser.h

	- Type.h

	- QueryParser.cpp

	- QueryParser.h

- - -

#### The utilities are listed below:

- Util/

	- Util.cpp (headers, macros, typedefs, functions...)
	
	- Util.h

	- Bstr.cpp (represent strings of arbitrary length)
	
	- Bstr.h (class, members and functions definitions)
	
	- Stream.cpp (store and use temp results, which may be very large)
	
	- Stream.h

	- Triple.cpp (deal with triples, a triple can be divided as subject(entity), predicate(entity), object(entity or literal))

	- Triple.h

	- BloomFilter.cpp

	- BloomFilter.h

	- ClassForVlistCache.h

	- VList.cpp

	- VList.h

- - -

#### The interface part is listed below:

- Server/ (client and server mode to use gStore)

	- Client.cpp

	- Client.h

	- Operation.cpp

	- Operation.h

	- Server.cpp

	- Server.h

	- Socket.cpp

	- Socket.h

	- client_http.hpp

	- server_http.hpp

- web/
	
	-

- - -

#### More details

To acquire a deep understanding of gStore codes, please go to [Code Detail](pdf/code_overview.pdf). See [use case](pdf/Gstore2.0_useCaseDoc.pdf) to understand the design of use cases, and see [OOA](pdf/OOA_class.pdf) and [OOD](pdf/OOD_class.pdf) for OOA design and OOD design, respectively.

If you want to know the sequence of a running gStore, please view the list below:

- [connect to server](jpg/A01-connectServer.jpg)

- [disconnect server](jpg/A02-disconnectServer.jpg)

- [load database](jpg/A03-loadDatabase.jpg)

- [unload database](jpg/A04-unloadDatabase.jpg)

- [create database](jpg/A05-buildDatabase.jpg)

- [delete database](jpg/A06-deleteDatabase.jpg)

- [connect to database](jpg/A07-connectDatabase.jpg)

- [disconnect database](jpg/A08-disconnectDatabase.jpg)

- [show databases](jpg/A09-showDatabase.jpg)

- [SPARQL query](jpg/A10-querySPARQL.jpg)

- [import RDF dataset](jpg/A11-loadRDF.jpg)

- [insert a triple](jpg/A12-insertRDF.jpg)

- [delete a triple](jpg/A13-deleteRDF.jpg)

- [create account](jpg/B01-createAccount.jpg)

- [delete account](jpg/B02-deleteAccount.jpg)

- [modify account authority](jpg/B03-changeAccount.jpg)

- [compulsively unload database](jpg/B04-removeDatabase.jpg)

- [see account authority](jpg/B05-showAccount.jpg)

It is really not strange to see something different with the original design in the source code. And some designed functions may have not be achieved so far.

- - -

#### Others

The api/ folder in gStore is used to store API program, libs and examples, please go to [API](API.md) for details. And test/ is used to store a series test programs or utilities, such as gtest, full_test and so on. Chapters related with test/ are [How To Use](USAGE.md) and [Test Result](TEST.md). This project need an ANTLR lib to parse the SPARQL query, whose code is placed in tools/(also archived here) and the compiled libantlr.a is placed in lib/ directory.

We place some datasets and queries in data/ directory as examples, and you can try them to see how gStore works. Related instructions are in [How To Use](USAGE.md). The docs/ directory contains all kinds of documents of gStore, including a series of markdown files and two folders, pdf/ and jpg/. Files whose type is pdf are placed in pdf/ folder, while files with jpg type are placed in jpg/ folder.

You are advised to start from the [README](../README.md) in the gStore root directory, and visit other chapters only when needed. At last, you will see all documents from link to link if you are really interested in gStore.

