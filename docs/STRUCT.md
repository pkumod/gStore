> gstore version: 1.0
>
> Last modification date: 2022-12-31
>
> Modified by: JianWang
>
> Modification description: Update source codes struct

**This chapter introduce the whole structure of the gStore system project.**

#### Figures

The whole architecture of gStore system is presented in [Architecture](png/system_architecture_en.png).
The thread model of 'ghttp' can be viewed in [EN](png/ghttp_thread_model_en.png) and [ZH](png/ghttp_thread_model_zh.png), which shows the relationship among main process, sever thread, query thread and so on.
The flow of answering a SPARQL query is given in [SPARQL Processing](png/sparql_processing.png), and the subprocess, which only targets at the BGP(Basic Graph Pattern) processing, is drawed in [BGP Processing](png/BGP.png).

---

#### The core source codes are listed below:

- Database/ (calling other core parts to deal with requests from interface part)
	
	- CSR.cpp

	- CSR.h

	- Database.cpp (achieve functions)

	- Database.h (class, members and functions definitions)
	
	- Executor.cpp

	- Executor.h

	- Join.cpp (join the node candidates to get results)

	- Join.h (class, members and functions definitions)

	- Optimizer.cpp
  
	- Optimizer.h
  
	- PlanGenerator.cpp

	- PlanGenerator.h
  
	- PlanTree.cpp
	
	- PlanTree.h
	
	- ResultTrigger.cpp
	
	- ResultTrigger.h
	
	- Statistics.cpp
	
	- Statistics.h 

	- Strategy.cpp 

	- Strategy.h
	
	- TableOperator.cpp
	
	- TableOperator.h
	
	- Txn_manager.cpp
	
	- Txn_manager.h

- GRPC/ (RPC server mode to use gStore)

	- APIUtil.cpp
	
	- APIUtil.h
	
	- grpc_content.cpp
	
	- grpc_content.h
	
	- grpc_message.cpp
	
	- grpc_message.h
	
	- grpc_noncopyable.h
	
	- grpc_operation.h
	
	- grpc_request_handler.h
	
	- grpc_router.cpp
	
	- grpc_router.h
	
	- grpc_routetable.cpp
	
	- grpc_routetable.h
	
	- grpc_server_task.cpp
	
	- grpc_server_task.h
	
	- grpc_server.cpp
	
	- grpc_server.h
	
	- grpc_status_code.cpp
	
	- grpc_status_code.h
	
	- grpc_stringpiece.h 

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
		
		- Tree.h 

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
    
	- topk/
    	- DPB/
        	- DynamicTrie.cpp
        	
			- DynamicTrie.h
        	
			- OrderedList.cpp
			
			- OrderedList.h
			
			- Pool.cpp
			
			- Pool.h
			
    	- DPBTopKUtil.cpp
    	
		- DPBTopKUtil.h
		
		- MinMaxHeap.hpp
		
		- TopKSearchPlan.cpp
		
		- TopKSearchPlan.h
		
		- TopKUtil.cpp
		
		- TopKUtil.h      

	- BasicQuery.cpp (basic type of queries without aggregate operations)

	- BasicQuery.h
	
	- BGPQuery.cpp
	
	- BGPQuery.h
	
	- GeneralEvaluation.cpp
	
	- GeneralEvaluation.h

	- IDList.cpp (candidate list of a node/variable in query)

	- IDList.h 
	
	- PathQueryHandler.cpp
	
	- PathQueryHandler.h
	
	- QueryCache.cpp
	
	- QueryCache.h
	
	- QueryTree.cpp
	
	- QueryTree.h
	
	- RegexExpression.h

	- ResultFilter.cpp

	- ResultFilter.h

	- ResultSet.cpp (keep the result set corresponding to a query)

	- ResultSet.h

	- SPARQLquery.cpp (deal with a entire SPARQL query)

	- SPARQLquery.h

	- TempResult.cpp

	- TempResult.h

	- Varset.cpp

	- Varset.h

- Signature/ (assign signatures for nodes and edges, but not for literals)

	- SigEntry.cpp

	- SigEntry.h

	- Signature.cpp

	- Signature.h

- StringIndex/

	- StringIndex.cpp
	
	- StringIndex.h

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
	
	- SPARQL/

		- SPARQLBaseListener.cpp (auto-generated, subtle modified manually, compressed)

    	- SPARQLBaseListener.h (auto-generated, subtle modified manually, compressed)

    	- SPARQLBaseVisitor.cpp (auto-generated, subtle modified manually, compressed)

    	- SPARQLBaseVisitor.h (auto-generated, subtle modified manually, compressed)
    	
		- SPARQLLexer.cpp (auto-generated, subtle modified manually, compressed)
    	
		- SPARQLLexer.h (auto-generated, subtle modified manually, compressed)
    	
		- SPARQLListener.cpp (auto-generated, subtle modified manually, compressed)
    	
		- SPARQLListener.h (auto-generated, subtle modified manually, compressed)
    	
		- SPARQLParser.cpp (auto-generated, subtle modified manually, compressed)
    	
		- SPARQLParser.h (auto-generated, subtle modified manually, compressed)
    	
		- SPARQLVisitor.cpp (auto-generated, subtle modified manually, compressed)
    	
		- SPARQLVisitor.h (auto-generated, subtle modified manually, compressed)

	- DBParser.cpp

	- DBParser.h

	- RDFParser.cpp

	- RDFParser.h

	- TurtleParser.cpp

	- TurtleParser.h

	- Type.h

	- QueryParser.cpp

	- QueryParser.h

- - -

#### The utilities are listed below:

- Util/

	- Bstr.cpp (represent strings of arbitrary length)
	
	- Bstr.h (class, members and functions definitions)

	- ClassForVlistCache.h

	- EvalMultitypeValue.cpp

	- EvalMultitypeValue.h
	
	- GraphLock.cpp
	
	- GraphLock.h
	
	- IDTriple.cpp
	
	- IDTriple.h
	
	- INIParser.cpp
	
	- INIParser.h
	
	- IPBlackList.cpp
	
	- IPBlackList.h
	
	- IPWhiteList.cpp
	
	- IPWhiteList.h
	
	- Latch.cpp
	
	- Latch.h
	
	- MD5.h
	
	- OrderedVector.cpp
	
	- OrderedVector.h
	
	- PrettyPrint.h
	
	- Slog.cpp
	
	- Slog.h
	
	- SpinLock.cpp
	
	- SpinLock.h
	
	- Stream.cpp (store and use temp results, which may be very large)
	
	- Stream.h
	
	- Transaction.cpp
	
	- Transaction.h

	- Triple.cpp (deal with triples, a triple can be divided as subject(entity), predicate(entity), object(entity or literal))

	- Triple.h
	
	- Util.cpp (headers, macros, typedefs, functions...)
	
	- Util.h

	- Version.cpp

	- Version.h

	- VList.cpp

	- VList.h
	
	- WebUrl.cpp
	
	- WebUrl.h

- - -

#### The interface part is listed below:

- Server/ (client and server mode to use gStore)
  
    - web/

	- Operation.cpp

	- Operation.h

	- Server.cpp

	- Server.h

	- Socket.cpp

	- Socket.h

	- client_http.hpp

	- server_http.hpp

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

