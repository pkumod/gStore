#help for make
#http://www.cnblogs.com/wang_yb/p/3990952.html
#https://segmentfault.com/a/1190000000349917
#http://blog.csdn.net/cuiyifang/article/details/7910268

#to use gprof to analyse efficience of the program:
#http://blog.chinaunix.net/uid-25194149-id-3215487.html

#to use gcov and lcov
#Notice that optimization should not be used here
#http://blog.163.com/bobile45@126/blog/static/96061992201382025729313/
#gcov -a main.cpp
#lcov --directory .   --capture --output-file dig.info 
#genhtml --output-directory . --frames --show-details dig.info 

#to use doxygen+graphviz+htmlhelp to generate document from source code:
#http://www.doxygen.nl/
#(also include good comments norm)
#http://blog.csdn.net/u010740725/article/details/51387810

#CXX=$(shell which clang 2>/dev/null || which gcc)
#ccache, readline, gcov lcov
#http://blog.csdn.net/u012421852/article/details/52138960
#
# How to speed up the compilation
# https://blog.csdn.net/a_little_a_day/article/details/78251928
# use make -j4, if error then use make utilizing only one thread
#use -j8 or higher may cause error
#http://blog.csdn.net/cscrazybing/article/details/50789482
#http://blog.163.com/liuhonggaono1@126/blog/static/10497901201210254622141/

#compile parameters

# WARN: maybe difficult to install ccache in some systems
#CXX = ccache g++
CXX = g++
CC = gcc

#the optimazition level of gcc/g++
#http://blog.csdn.net/hit_090420216/article/details/44900215
#NOTICE: -O2 is recommended, while -O3(add loop-unroll and inline-function) is dangerous
#when developing, not use -O because it will disturb the normal 
#routine. use it for test and release.
CFLAGS = -c -Wall -O2 -pthread -std=c++11
EXEFLAG = -O2 -pthread -std=c++11
#-coverage for debugging
# CFLAGS = -c -Wall -pthread -g3 -std=c++11  -gdwarf-2
# EXEFLAG = -pthread -g3 -std=c++11 -gdwarf-2
#-coverage for debugging and with performance
# CFLAGS = -c -Wall -pthread -g3 -std=c++11  -gdwarf-2 -pg
# EXEFLAG = -pthread -g3 -std=c++11 -gdwarf-2 -pg

#add -lreadline [-ltermcap] if using readline or objs contain readline
# library = -lreadline -L./lib -L/usr/local/lib -lantlr -lgcov -lboost_thread -lboost_filesystem -lboost_system -lboost_regex -lpthread -I/usr/local/include/boost -lcurl
#library = -lreadline -L./lib -L/usr/local/lib -L/usr/lib/ -L./workflow-nossl/_lib -L./workflow-nossl/_include -lantlr4-runtime -lgcov -lboost_thread -lboost_filesystem -lboost_system -lboost_regex -lpthread -I/usr/local/include/boost -lcurl -lworkflow -llog4cplus
#library = -lreadline -L./lib -L/usr/local/lib -L/usr/lib/  -L./tools/workflow-master/_lib -L./tools/workflow-master/_include  -lantlr4-runtime -lgcov -lboost_thread -lboost_filesystem -lboost_system -lboost_regex -lpthread -I/usr/local/include/boost -lcurl  -llog4cplus -lworkflow
#library = -lreadline -L./lib -L/usr/local/lib -L/usr/lib/ -lantlr4-runtime -lgcov -lboost_thread -lboost_filesystem -lboost_system -lboost_regex -lpthread -I/usr/local/include/boost -lcurl -llog4cplus -Wl,-rpath='/usr/local/lib'
library = -lreadline -L./lib -L/usr/local/lib -L/usr/lib/ -lantlr4-runtime -lgcov -lboost_thread -lboost_system -lboost_regex -lpthread -I/usr/local/include/boost -lcurl -llog4cplus
#used for parallelsort
#used for parallelsort
march = -march=native
openmp = -fopenmp ${march}
# library = -ltermcap -lreadline -L./lib -lantlr -lgcov
def64IO = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -D_LARGEFILE_SOURCE
# load dynamic lib
ldl = -ldl
# paths

objdir = .objs/

exedir = bin/

testdir = scripts/

lib_antlr = lib/libantlr4-runtime.a

lib_rpc = lib/libworkflow.a

lib_log = lib/liblog4cplus.a

api_cpp = api/http/cpp/lib/libgstoreconnector.a

api_socket = api/socket/cpp/lib/libclient.a

# api_java = api/http/java/lib/GstoreJavaAPI.jar

# objects

#sstreeobj = $(objdir)Tree.o $(objdir)Storage.o $(objdir)Node.o $(objdir)IntlNode.o $(objdir)LeafNode.o $(objdir)Heap.o 
sitreeobj = $(objdir)SITree.o $(objdir)SIStorage.o $(objdir)SINode.o $(objdir)SIIntlNode.o $(objdir)SILeafNode.o $(objdir)SIHeap.o 
istreeobj = $(objdir)ISTree.o $(objdir)ISStorage.o $(objdir)ISNode.o $(objdir)ISIntlNode.o $(objdir)ISLeafNode.o $(objdir)ISHeap.o 
ivtreeobj = $(objdir)IVTree.o $(objdir)IVStorage.o $(objdir)IVNode.o $(objdir)IVIntlNode.o $(objdir)IVLeafNode.o $(objdir)IVHeap.o 
ivarrayobj = $(objdir)IVArray.o $(objdir)IVEntry.o $(objdir)IVBlockManager.o
isarrayobj = $(objdir)ISArray.o $(objdir)ISEntry.o $(objdir)ISBlockManager.o

kvstoreobj = $(objdir)KVstore.o $(sitreeobj) $(istreeobj) $(ivtreeobj) $(ivarrayobj) $(isarrayobj) #$(sstreeobj)

utilobj = $(objdir)Slog.o $(objdir)Util.o $(objdir)Bstr.o $(objdir)Stream.o $(objdir)Triple.o $(objdir)VList.o \
			$(objdir)EvalMultitypeValue.o $(objdir)IDTriple.o $(objdir)Version.o $(objdir)Transaction.o $(objdir)Latch.o $(objdir)IPWhiteList.o \
			$(objdir)IPBlackList.o  $(objdir)SpinLock.o $(objdir)GraphLock.o $(objdir)WebUrl.o $(objdir)INIParser.o $(objdir)OrderedVector.o

topkobj = $(objdir)DynamicTrie.o $(objdir)OrderedList.o $(objdir)Pool.o $(objdir)TopKUtil.o $(objdir)DPBTopKUtil.o $(objdir)TopKSearchPlan.o

queryobj = $(objdir)SPARQLquery.o $(objdir)BasicQuery.o $(objdir)ResultSet.o  $(objdir)IDList.o $(objdir)QueryPlan.o\
		   $(objdir)Varset.o $(objdir)QueryTree.o $(objdir)TempResult.o $(objdir)QueryCache.o $(objdir)GeneralEvaluation.o \
		   $(objdir)PathQueryHandler.o $(objdir)BGPQuery.o $(topkobj)

signatureobj = $(objdir)SigEntry.o $(objdir)Signature.o

vstreeobj = $(objdir)VSTree.o $(objdir)EntryBuffer.o $(objdir)LRUCache.o $(objdir)VNode.o

stringindexobj = $(objdir)StringIndex.o

parserobj = $(objdir)RDFParser.o $(objdir)SPARQLParser.o \
			$(objdir)SPARQLLexer.o $(objdir)TurtleParser.o $(objdir)QueryParser.o

serverobj = $(objdir)Operation.o $(objdir)Server.o $(objdir)Socket.o

grpcobj = $(objdir)grpc_server.o $(objdir)grpc_server_task.o $(objdir)grpc_message.o \
		  $(objdir)grpc_router.o $(objdir)grpc_routetable.o $(objdir)grpc_content.o \
		  $(objdir)grpc_status_code.o ${objdir}APIUtil.o

databaseobj = $(objdir)Statistics.o $(objdir)Database.o $(objdir)Join.o $(objdir)Strategy.o \
 $(objdir)CSR.o $(objdir)Txn_manager.o $(objdir)TableOperator.o $(objdir)PlanTree.o  \
 $(objdir)PlanGenerator.o $(objdir)Executor.o $(objdir)Optimizer.o $(objdir)ResultTrigger.o

trieobj = $(objdir)Trie.o $(objdir)TrieNode.o

objfile = $(kvstoreobj) $(vstreeobj) $(stringindexobj) $(parserobj) $(serverobj) $(databaseobj) \
		  $(utilobj) $(signatureobj) $(queryobj) $(trieobj)
	 
inc = -I./tools/antlr4-cpp-runtime-4/runtime/src
inc_rpc = -I./tools/workflow/_include
inc_log = -I./tools/log4cplus/include

#auto generate dependencies
# http://blog.csdn.net/gmpy_tiger/article/details/51849474
# http://blog.csdn.net/jeffrey0000/article/details/12421317

#gtest

TARGET = $(exedir)gexport $(exedir)gbuild $(exedir)gserver $(exedir)gserver_backup_scheduler $(exedir)gquery \
         $(exedir)gadd $(exedir)gsub $(exedir)ghttp  $(exedir)gmonitor $(exedir)gshow \
		 $(exedir)shutdown $(exedir)ginit $(exedir)gdrop $(testdir)update_test $(testdir)dataset_test \
		 $(testdir)transaction_test $(testdir)run_transaction $(testdir)workload $(testdir)debug_test $(exedir)gbackup \
		 $(exedir)grestore $(exedir)gpara $(exedir)rollback $(exedir)grpc $(exedir)gconsole
# TARGET = $(exedir)gbuild $(exedir)gdrop $(exedir)gquery $(exedir)ginit

all: $(TARGET)
	@echo "Compilation ends successfully!"
	@bash scripts/init.sh

#BETTER: use for loop to reduce the lines
#NOTICE: g++ -MM will run error if linking failed, like Database.h/../SparlParser.h/../antlr3.h

#executables begin

#NOTICE:not include g*.o in objfile due to multiple definitions of main()

$(exedir)gexport: $(lib_antlr) $(objdir)gexport.o $(objfile)
	$(CXX) $(EXEFLAG) -o $(exedir)gexport $(objdir)gexport.o $(objfile) $(library) $(openmp) ${ldl}

$(exedir)gdrop: $(lib_antlr) $(objdir)gdrop.o $(objfile)
	$(CXX) $(EXEFLAG) -o $(exedir)gdrop $(objdir)gdrop.o $(objfile) $(library) $(openmp) ${ldl}

$(exedir)ginit: $(lib_antlr) $(objdir)ginit.o $(objfile)
	$(CXX) $(EXEFLAG) -o $(exedir)ginit $(objdir)ginit.o $(objfile) $(library) $(openmp) ${ldl}

$(exedir)shutdown: $(lib_antlr) $(objdir)shutdown.o $(objfile) $(api_cpp)
	$(CXX) $(EXEFLAG) -o $(exedir)shutdown $(objdir)shutdown.o $(objfile) $(openmp) -L./api/http/cpp/lib -lgstoreconnector $(library) ${ldl}

$(exedir)gmonitor: $(lib_antlr) $(objdir)gmonitor.o $(objfile)
	$(CXX) $(EXEFLAG) -o $(exedir)gmonitor $(objdir)gmonitor.o $(objfile) $(library) $(openmp) ${ldl}

$(exedir)gshow: $(lib_antlr) $(objdir)gshow.o $(objfile)
	$(CXX) $(EXEFLAG) -o $(exedir)gshow $(objdir)gshow.o $(objfile) $(library) $(openmp) ${ldl}

$(exedir)gbuild: $(lib_antlr) $(objdir)gbuild.o $(objfile) 
	$(CXX) $(EXEFLAG) -o $(exedir)gbuild $(objdir)gbuild.o $(objfile) $(library) $(openmp) ${ldl}

$(exedir)gquery: $(lib_antlr) $(objdir)gquery.o $(objfile) 
	$(CXX) $(EXEFLAG) -o $(exedir)gquery $(objdir)gquery.o $(objfile) $(library) $(openmp) ${ldl}

$(exedir)gserver: $(lib_antlr) $(objdir)gserver.o $(objfile) 
	$(CXX) $(EXEFLAG) -o $(exedir)gserver $(objdir)gserver.o $(objfile) $(library) $(openmp) ${ldl}

$(exedir)gserver_backup_scheduler: $(lib_antlr) $(objdir)gserver_backup_scheduler.o $(objfile)
	$(CXX) $(EXEFLAG) -o $(exedir)gserver_backup_scheduler $(objdir)gserver_backup_scheduler.o $(objfile) $(library) $(openmp) ${ldl}

$(exedir)ghttp: $(lib_antlr) $(objdir)ghttp.o ./Server/server_http.hpp ./Server/client_http.hpp $(objfile) ${objdir}APIUtil.o
	$(CXX) $(EXEFLAG) -o $(exedir)ghttp $(objdir)ghttp.o $(objfile) ${objdir}APIUtil.o $(library) $(inc) -DUSE_BOOST_REGEX $(openmp) ${ldl}

#$(exedir)gapiserver: $(lib_antlr) $(lib_workflow) $(objdir)gapiserver.o  $(objfile)
#	$(CXX) $(EXEFLAG) -o $(exedir)gapiserver $(objdir)gapiserver.o $(objfile) $(library) $(openmp)

$(exedir)grpc: $(lib_antlr) $(lib_rpc) $(objdir)grpc.o $(grpcobj) $(objfile)
	$(CXX) $(EXEFLAG) -o $(exedir)grpc $(objdir)grpc.o ${grpcobj} $(objfile) $(library) $(inc) ${inc_rpc} -lworkflow -lssl -lcrypto $(openmp) ${ldl}

$(exedir)gbackup: $(lib_antlr) $(objdir)gbackup.o $(objfile)
	$(CXX) $(EXEFLAG) -o $(exedir)gbackup $(objdir)gbackup.o $(objfile) $(library) $(openmp) ${ldl}

$(exedir)grestore: $(lib_antlr) $(objdir)grestore.o $(objfile)
	$(CXX) $(EXEFLAG) -o $(exedir)grestore $(objdir)grestore.o $(objfile) $(library) $(openmp) ${ldl}

$(exedir)gpara: $(lib_antlr) $(objdir)gpara.o $(objfile)
	$(CXX) $(EXEFLAG) -o $(exedir)gpara $(objdir)gpara.o $(objfile) $(library) $(openmp) -L./api/http/cpp/lib -lgstoreconnector $(library) ${ldl}

$(exedir)rollback: $(lib_antlr) $(objdir)rollback.o $(objfile)
	$(CXX) $(EXEFLAG) -o $(exedir)rollback $(objdir)rollback.o $(objfile) $(library) $(openmp) -L./api/http/cpp/lib -lgstoreconnector $(library) ${ldl}

$(testdir)update_test: $(lib_antlr) $(objdir)update_test.o $(objfile)
	$(CXX) $(EXEFLAG) -o $(testdir)update_test $(objdir)update_test.o $(objfile) $(library) $(openmp) ${ldl}

$(testdir)dataset_test: $(lib_antlr) $(objdir)dataset_test.o $(objfile)
	$(CXX) $(EXEFLAG) -o $(testdir)dataset_test $(objdir)dataset_test.o $(objfile) $(library) $(openmp) ${ldl}

$(testdir)transaction_test: $(lib_antlr) $(objdir)transaction_test.o $(objfile)
	$(CXX) $(EXEFLAG) -o $(testdir)transaction_test $(objdir)transaction_test.o $(objfile) $(library) $(openmp) ${ldl}

$(testdir)run_transaction: $(lib_antlr) $(objdir)run_transaction.o $(objfile)
	$(CXX) $(EXEFLAG) -o $(testdir)run_transaction $(objdir)run_transaction.o $(objfile) $(library) $(openmp) -L./api/http/cpp/lib -lgstoreconnector $(library) ${ldl}

$(testdir)workload: $(lib_antlr) $(objdir)workload.o $(objfile)
	$(CXX) $(EXEFLAG) -o $(testdir)workload $(objdir)workload.o $(objfile) $(library) $(openmp) ${ldl}

$(testdir)debug_test: $(lib_antlr) $(objdir)debug_test.o $(objfile)
	$(CXX) $(EXEFLAG) -o $(testdir)debug_test $(objdir)debug_test.o $(objfile) $(library) $(openmp) ${ldl}

$(exedir)gconsole: $(lib_antlr) $(objdir)gconsole.o $(objfile) 
	$(CXX) $(EXEFLAG) -o $(exedir)gconsole $(objdir)gconsole.o $(objfile) $(library) $(openmp) ${ldl}

#executables end


#objects in Main/ begin

$(objdir)gexport.o: Main/gexport.cpp Database/Database.h Util/Util.h $(lib_antlr)
	$(CXX) $(CFLAGS) Main/gexport.cpp $(inc) $(inc_log) -o $(objdir)gexport.o $(openmp)

$(objdir)gdrop.o: Main/gdrop.cpp Database/Database.h Util/Util.h $(lib_antlr)
	$(CXX) $(CFLAGS) Main/gdrop.cpp $(inc) $(inc_log) -o $(objdir)gdrop.o $(openmp)

$(objdir)ginit.o: Main/ginit.cpp Database/Database.h Util/Util.h $(lib_antlr)
	$(CXX) $(CFLAGS) Main/ginit.cpp $(inc) $(inc_log) -o $(objdir)ginit.o $(openmp)

$(objdir)shutdown.o: Main/shutdown.cpp Database/Database.h Util/Util.h $(lib_antlr)
	$(CXX) $(CFLAGS)	Main/shutdown.cpp $(inc) $(inc_log) -o $(objdir)shutdown.o $(openmp)

$(objdir)gmonitor.o: Main/gmonitor.cpp Database/Database.h Util/Util.h $(lib_antlr)
	$(CXX) $(CFLAGS) Main/gmonitor.cpp $(inc) $(inc_log) -o $(objdir)gmonitor.o $(openmp)

$(objdir)gshow.o: Main/gshow.cpp Database/Database.h Util/Util.h $(lib_antlr)
	$(CXX) $(CFLAGS) Main/gshow.cpp $(inc) $(inc_log) -o $(objdir)gshow.o $(openmp)

$(objdir)gbuild.o: Main/gbuild.cpp Database/Database.h Util/Util.h $(lib_antlr)
	$(CXX) $(CFLAGS) Main/gbuild.cpp $(inc) $(inc_log) -o $(objdir)gbuild.o $(openmp)
	
$(objdir)gquery.o: Main/gquery.cpp Database/Database.h Util/Util.h $(lib_antlr)
	$(CXX) $(CFLAGS) Main/gquery.cpp $(inc) $(inc_log) -o $(objdir)gquery.o $(openmp) #-DREADLINE_ON
	#add -DREADLINE_ON if using readline

$(objdir)gserver.o: Main/gserver.cpp Server/Server.h Util/Util.h $(lib_antlr)
	$(CXX) $(CFLAGS) Main/gserver.cpp $(inc) $(inc_log) -o $(objdir)gserver.o $(openmp)

$(objdir)gserver_backup_scheduler.o: Main/gserver_backup_scheduler.cpp Server/Server.h Util/Util.h $(lib_antlr)
	$(CXX) $(CFLAGS) Main/gserver_backup_scheduler.cpp $(inc) $(inc_log) -o $(objdir)gserver_backup_scheduler.o $(openmp)

$(objdir)ghttp.o: Main/ghttp.cpp Server/server_http.hpp Server/client_http.hpp Database/Database.h Database/Txn_manager.h Util/Util.h Util/IPWhiteList.h Util/IPBlackList.h $(lib_antlr) Util/INIParser.h Util/WebUrl.h GRPC/APIUtil.h
	$(CXX) $(CFLAGS) Main/ghttp.cpp $(inc) $(inc_log) -o $(objdir)ghttp.o -DUSE_BOOST_REGEX $(def64IO) $(openmp)

#$(objdir)gapiserver.o: Main/gapiserver.cpp Database/Database.h Database/Txn_manager.h Util/Util.h Util/Util_New.h Util/IPWhiteList.h Util/IPBlackList.h Util/WebUrl.h  $(lib_antlr) $(lib_workflow)
#	$(CXX) $(CFLAGS) Main/gapiserver.cpp $(inc) $(inc_workflow) -o $(objdir)gapiserver.o $(openmp)

$(objdir)grpc.o: Main/grpc.cpp GRPC/grpc_server.h GRPC/grpc_status_code.h GRPC/grpc_operation.h GRPC/APIUtil.h Database/Database.h Database/Txn_manager.h Util/Util.h $(lib_antlr) $(lib_rpc)
	$(CXX) $(CFLAGS) Main/grpc.cpp $(inc) $(inc_log) $(inc_rpc) -o $(objdir)grpc.o $(def64IO) $(openmp)

$(objdir)gbackup.o: Main/gbackup.cpp Database/Database.h Util/Util.h $(lib_antlr)
	$(CXX) $(CFLAGS) Main/gbackup.cpp $(inc) $(inc_log) -o $(objdir)gbackup.o $(openmp)

$(objdir)grestore.o: Main/grestore.cpp Database/Database.h Util/Util.h $(lib_antlr)
	$(CXX) $(CFLAGS) Main/grestore.cpp $(inc) $(inc_log) -o $(objdir)grestore.o $(openmp)

$(objdir)gpara.o: Main/gpara.cpp Database/Database.h Util/Util.h $(lib_antlr)
	$(CXX) $(CFLAGS) Main/gpara.cpp $(inc) $(inc_log) -o $(objdir)gpara.o $(openmp)

$(objdir)rollback.o: Main/rollback.cpp Database/Database.h Util/Util.h $(lib_antlr)
	$(CXX) $(CFLAGS) Main/rollback.cpp $(inc) $(inc_log) -o $(objdir)rollback.o $(openmp)

$(objdir)gconsole.o: Main/gconsole.cpp Database/Database.h Util/Util.h $(lib_antlr)
	$(CXX) $(CFLAGS) Main/gconsole.cpp $(inc) $(inc_log) -o $(objdir)gconsole.o $(openmp)
#objects in Main/ end

#objects in scripts/ begin

$(objdir)update_test.o: $(testdir)update_test.cpp Database/Database.h Util/Util.h $(lib_antlr)
	$(CXX) $(CFLAGS) $(testdir)update_test.cpp $(inc) $(inc_log) -o $(objdir)update_test.o $(openmp)

$(objdir)dataset_test.o: $(testdir)dataset_test.cpp Database/Database.h Util/Util.h $(lib_antlr)
	$(CXX) $(CFLAGS) $(testdir)dataset_test.cpp $(inc) $(inc_log) -o $(objdir)dataset_test.o $(openmp)

$(objdir)transaction_test.o: $(testdir)transaction_test.cpp Database/Database.h Database/Txn_manager.h Util/Util.h $(lib_antlr)
	$(CXX) $(CFLAGS) $(testdir)transaction_test.cpp $(inc) $(inc_log) -o $(objdir)transaction_test.o $(openmp)

$(objdir)run_transaction.o: $(testdir)run_transaction.cpp Util/Util.h $(lib_antlr)
	$(CXX) $(CFLAGS) $(testdir)run_transaction.cpp $(inc) $(inc_log) -o $(objdir)run_transaction.o $(openmp)

$(objdir)workload.o: $(testdir)workload.cpp Util/Util.h $(lib_antlr)
	$(CXX) $(CFLAGS) $(testdir)workload.cpp $(inc) $(inc_log) -o $(objdir)workload.o $(openmp)

$(objdir)debug_test.o: $(testdir)debug_test.cpp Util/Util.h $(lib_antlr)
	$(CXX) $(CFLAGS) $(testdir)debug_test.cpp $(inc) $(inc_log) -o $(objdir)debug_test.o $(openmp)

#objects in scripts/ end


#objects in kvstore/ begin

#objects in sstree/ begin
#$(objdir)Tree.o: KVstore/SSTree/Tree.cpp KVstore/SSTree/Tree.h $(objdir)Stream.o
	#$(CXX) $(CFLAGS) KVstore/SSTree/Tree.cpp -o $(objdir)Tree.o

#$(objdir)Storage.o: KVstore/SSTree/storage/Storage.cpp KVstore/SSTree/storage/Storage.h $(objdir)Util.o
	#$(CXX) $(CFLAGS) KVstore/SSTree/storage/Storage.cpp -o $(objdir)Storage.o $(def64IO)

#$(objdir)Node.o: KVstore/SSTree/node/Node.cpp KVstore/SSTree/node/Node.h $(objdir)Util.o
	#$(CXX) $(CFLAGS) KVstore/SSTree/node/Node.cpp -o $(objdir)Node.o

#$(objdir)IntlNode.o: KVstore/SSTree/node/IntlNode.cpp KVstore/SSTree/node/IntlNode.h
	#$(CXX) $(CFLAGS) KVstore/SSTree/node/IntlNode.cpp -o $(objdir)IntlNode.o

#$(objdir)LeafNode.o: KVstore/SSTree/node/LeafNode.cpp KVstore/SSTree/node/LeafNode.h
	#$(CXX) $(CFLAGS) KVstore/SSTree/node/LeafNode.cpp -o $(objdir)LeafNode.o

#$(objdir)Heap.o: KVstore/SSTree/heap/Heap.cpp KVstore/SSTree/heap/Heap.h $(objdir)Util.o
	#$(CXX) $(CFLAGS) KVstore/SSTree/heap/Heap.cpp -o $(objdir)Heap.o
#objects in sstree/ end

#objects in sitree/ begin
$(objdir)SITree.o: KVstore/SITree/SITree.cpp KVstore/SITree/SITree.h $(objdir)Stream.o
	$(CXX) $(CFLAGS) KVstore/SITree/SITree.cpp $(inc_log) -o $(objdir)SITree.o $(openmp)

$(objdir)SIStorage.o: KVstore/SITree/storage/SIStorage.cpp KVstore/SITree/storage/SIStorage.h $(objdir)Util.o
	$(CXX) $(CFLAGS) KVstore/SITree/storage/SIStorage.cpp $(inc_log) -o $(objdir)SIStorage.o $(def64IO) $(openmp)

$(objdir)SINode.o: KVstore/SITree/node/SINode.cpp KVstore/SITree/node/SINode.h $(objdir)Util.o
	$(CXX) $(CFLAGS) KVstore/SITree/node/SINode.cpp $(inc_log) -o $(objdir)SINode.o $(openmp)

$(objdir)SIIntlNode.o: KVstore/SITree/node/SIIntlNode.cpp KVstore/SITree/node/SIIntlNode.h
	$(CXX) $(CFLAGS) KVstore/SITree/node/SIIntlNode.cpp $(inc_log) -o $(objdir)SIIntlNode.o $(openmp)

$(objdir)SILeafNode.o: KVstore/SITree/node/SILeafNode.cpp KVstore/SITree/node/SILeafNode.h
	$(CXX) $(CFLAGS) KVstore/SITree/node/SILeafNode.cpp $(inc_log) -o $(objdir)SILeafNode.o $(openmp)

$(objdir)SIHeap.o: KVstore/SITree/heap/SIHeap.cpp KVstore/SITree/heap/SIHeap.h $(objdir)Util.o
	$(CXX) $(CFLAGS) KVstore/SITree/heap/SIHeap.cpp $(inc_log) -o $(objdir)SIHeap.o $(openmp)
#objects in sitree/ end

#objects in istree/ begin
$(objdir)ISTree.o: KVstore/ISTree/ISTree.cpp KVstore/ISTree/ISTree.h $(objdir)Stream.o
	$(CXX) $(CFLAGS) KVstore/ISTree/ISTree.cpp $(inc_log) -o $(objdir)ISTree.o $(openmp)

$(objdir)ISStorage.o: KVstore/ISTree/storage/ISStorage.cpp KVstore/ISTree/storage/ISStorage.h $(objdir)Util.o
	$(CXX) $(CFLAGS) KVstore/ISTree/storage/ISStorage.cpp $(inc_log) -o $(objdir)ISStorage.o $(def64IO) $(openmp)

$(objdir)ISNode.o: KVstore/ISTree/node/ISNode.cpp KVstore/ISTree/node/ISNode.h $(objdir)Util.o
	$(CXX) $(CFLAGS) KVstore/ISTree/node/ISNode.cpp $(inc_log) -o $(objdir)ISNode.o $(openmp)

$(objdir)ISIntlNode.o: KVstore/ISTree/node/ISIntlNode.cpp KVstore/ISTree/node/ISIntlNode.h
	$(CXX) $(CFLAGS) KVstore/ISTree/node/ISIntlNode.cpp $(inc_log) -o $(objdir)ISIntlNode.o $(openmp)

$(objdir)ISLeafNode.o: KVstore/ISTree/node/ISLeafNode.cpp KVstore/ISTree/node/ISLeafNode.h
	$(CXX) $(CFLAGS) KVstore/ISTree/node/ISLeafNode.cpp $(inc_log) -o $(objdir)ISLeafNode.o $(openmp)

$(objdir)ISHeap.o: KVstore/ISTree/heap/ISHeap.cpp KVstore/ISTree/heap/ISHeap.h $(objdir)Util.o
	$(CXX) $(CFLAGS) KVstore/ISTree/heap/ISHeap.cpp $(inc_log) -o $(objdir)ISHeap.o $(openmp)
#objects in istree/ end

#objects in isarray/ begin
$(objdir)ISArray.o: KVstore/ISArray/ISArray.cpp KVstore/ISArray/ISArray.h $(objdir)VList.o
	$(CXX) $(CFLAGS) KVstore/ISArray/ISArray.cpp $(inc_log) -o $(objdir)ISArray.o

$(objdir)ISBlockManager.o: KVstore/ISArray/ISBlockManager.cpp KVstore/ISArray/ISBlockManager.h 
	$(CXX) $(CFLAGS) KVstore/ISArray/ISBlockManager.cpp $(inc_log) -o $(objdir)ISBlockManager.o

$(objdir)ISEntry.o: KVstore/ISArray/ISEntry.cpp KVstore/ISArray/ISEntry.h
	$(CXX) $(CFLAGS) KVstore/ISArray/ISEntry.cpp $(inc_log) -o $(objdir)ISEntry.o
#objects in isarray/ end

#objects in ivtree/ begin
$(objdir)IVTree.o: KVstore/IVTree/IVTree.cpp KVstore/IVTree/IVTree.h $(objdir)Stream.o $(objdir)VList.o
	$(CXX) $(CFLAGS) KVstore/IVTree/IVTree.cpp $(inc_log) -o $(objdir)IVTree.o $(openmp)

$(objdir)IVStorage.o: KVstore/IVTree/storage/IVStorage.cpp KVstore/IVTree/storage/IVStorage.h $(objdir)Util.o
	$(CXX) $(CFLAGS) KVstore/IVTree/storage/IVStorage.cpp $(inc_log) -o $(objdir)IVStorage.o $(def64IO) $(openmp)

$(objdir)IVNode.o: KVstore/IVTree/node/IVNode.cpp KVstore/IVTree/node/IVNode.h $(objdir)Util.o
	$(CXX) $(CFLAGS) KVstore/IVTree/node/IVNode.cpp $(inc_log) -o $(objdir)IVNode.o $(openmp)

$(objdir)IVIntlNode.o: KVstore/IVTree/node/IVIntlNode.cpp KVstore/IVTree/node/IVIntlNode.h
	$(CXX) $(CFLAGS) KVstore/IVTree/node/IVIntlNode.cpp $(inc_log) -o $(objdir)IVIntlNode.o $(openmp)

$(objdir)IVLeafNode.o: KVstore/IVTree/node/IVLeafNode.cpp KVstore/IVTree/node/IVLeafNode.h
	$(CXX) $(CFLAGS) KVstore/IVTree/node/IVLeafNode.cpp $(inc_log) -o $(objdir)IVLeafNode.o $(openmp)

$(objdir)IVHeap.o: KVstore/IVTree/heap/IVHeap.cpp KVstore/IVTree/heap/IVHeap.h $(objdir)Util.o
	$(CXX) $(CFLAGS) KVstore/IVTree/heap/IVHeap.cpp $(inc_log) -o $(objdir)IVHeap.o $(openmp)
#objects in ivtree/ end

#objects in ivarray/ begin
$(objdir)IVArray.o: KVstore/IVArray/IVArray.cpp KVstore/IVArray/IVArray.h $(objdir)VList.o  $(objdir)Transaction.o 
	$(CXX) $(CFLAGS) KVstore/IVArray/IVArray.cpp $(inc_log) -o $(objdir)IVArray.o

$(objdir)IVBlockManager.o: KVstore/IVArray/IVBlockManager.cpp KVstore/IVArray/IVBlockManager.h 
	$(CXX) $(CFLAGS) KVstore/IVArray/IVBlockManager.cpp $(inc_log) -o $(objdir)IVBlockManager.o

$(objdir)IVEntry.o: KVstore/IVArray/IVEntry.cpp KVstore/IVArray/IVEntry.h $(objdir)Version.o $(objdir)GraphLock.o
	$(CXX) $(CFLAGS) KVstore/IVArray/IVEntry.cpp $(inc_log) -o $(objdir)IVEntry.o

#objects in ivarray/ end

$(objdir)KVstore.o: KVstore/KVstore.cpp KVstore/KVstore.h KVstore/Tree.h 
	$(CXX) $(CFLAGS) KVstore/KVstore.cpp $(inc) $(inc_log) -o $(objdir)KVstore.o $(openmp)

#objects in kvstore/ end


#objects in Database/ begin

$(objdir)Statistics.o: Database/Statistics.cpp Database/Statistics.h $(objdir)Util.o $(objdir)KVstore.o
	$(CXX) $(CFLAGS) Database/Statistics.cpp $(inc) $(inc_log) -o $(objdir)Statistics.o $(openmp)

$(objdir)Database.o: Database/Database.cpp Database/Database.h \
	$(objdir)IDList.o $(objdir)ResultSet.o $(objdir)SPARQLquery.o \
	$(objdir)BasicQuery.o $(objdir)Triple.o $(objdir)SigEntry.o \
	$(objdir)KVstore.o $(objdir)VSTree.o $(objdir)Statistics.o \
	$(objdir)Util.o $(objdir)RDFParser.o $(objdir)Join.o $(objdir)GeneralEvaluation.o $(objdir)StringIndex.o $(objdir)Transaction.o
	$(CXX) $(CFLAGS) Database/Database.cpp $(inc) $(inc_log) -o $(objdir)Database.o $(openmp)

$(objdir)Join.o: Database/Join.cpp Database/Join.h $(objdir)IDList.o $(objdir)BasicQuery.o $(objdir)Util.o\
	$(objdir)KVstore.o $(objdir)Util.o $(objdir)SPARQLquery.o $(objdir)Transaction.o
	$(CXX) $(CFLAGS) Database/Join.cpp $(inc) $(inc_log) -o $(objdir)Join.o $(openmp)

$(objdir)Strategy.o: Database/Strategy.cpp Database/Strategy.h $(objdir)SPARQLquery.o $(objdir)BasicQuery.o \
	$(objdir)Triple.o $(objdir)IDList.o $(objdir)KVstore.o $(objdir)VSTree.o $(objdir)Util.o $(objdir)Join.o $(objdir)Transaction.o
	$(CXX) $(CFLAGS) Database/Strategy.cpp $(inc) $(inc_log) -o $(objdir)Strategy.o $(openmp)

$(objdir)CSR.o: Database/CSR.cpp Database/CSR.h
	$(CXX) $(CFLAGS) Database/CSR.cpp $(inc) -o $(objdir)CSR.o $(openmp)
	$(CXX) -std=c++11 -fPIC -shared Database/CSR.cpp -o lib/libgcsr.so

$(objdir)TableOperator.o: Database/TableOperator.cpp Database/TableOperator.h $(objdir)Util.o $(objdir)BasicQuery.o
	$(CXX) $(CFLAGS) Database/TableOperator.cpp $(inc) $(inc_log) -o $(objdir)TableOperator.o $(openmp)

$(objdir)ResultTrigger.o: Database/ResultTrigger.cpp Database/ResultTrigger.h $(objdir)Util.o
	$(CXX) $(CFLAGS) Database/ResultTrigger.cpp $(inc) $(inc_log) -o $(objdir)ResultTrigger.o $(openmp)

$(objdir)PlanTree.o: Database/PlanTree.cpp Database/PlanTree.h $(objdir)BasicQuery.o $(objdir)TableOperator.o \
    $(objdir)BGPQuery.o $(objdir)Util.o
	$(CXX) $(CFLAGS) Database/PlanTree.cpp $(inc) $(inc_log) -o $(objdir)PlanTree.o  $(openmp)

$(objdir)PlanGenerator.o: Database/PlanGenerator.cpp Database/PlanGenerator.h \
	$(objdir)Util.o $(objdir)BasicQuery.o $(objdir)BGPQuery.o $(objdir)IDList.o $(objdir)KVstore.o \
	$(objdir)Statistics.o $(objdir)PlanTree.o $(objdir)TableOperator.o $(objdir)OrderedVector.o
	$(CXX) $(CFLAGS) Database/PlanGenerator.cpp $(inc) $(inc_log) -o $(objdir)PlanGenerator.o $(openmp)

$(objdir)Executor.o: Database/Executor.cpp Database/Executor.h $(objdir)Util.o $(objdir)SPARQLquery.o $(objdir)BasicQuery.o $(objdir)IDList.o \
	$(objdir)KVstore.o  $(objdir)VSTree.o $(objdir)Join.o $(objdir)Transaction.o $(objdir)TableOperator.o $(objdir)ResultTrigger.o \
	$(objdir)QueryPlan.o $(objdir)Statistics.o $(objdir)PlanTree.o $(objdir)PlanGenerator.o $(objdir)OrderedVector.o $(objdir)DPBTopKUtil.o
	$(CXX) $(CFLAGS) Database/Executor.cpp $(inc) $(inc_log) -o $(objdir)Executor.o $(openmp) ${ldl}

$(objdir)Optimizer.o: Database/Optimizer.cpp Database/Optimizer.h $(objdir)Util.o $(objdir)SPARQLquery.o $(objdir)IDList.o \
	$(objdir)KVstore.o  $(objdir)VSTree.o $(objdir)Join.o $(objdir)Transaction.o $(objdir)TableOperator.o $(objdir)ResultTrigger.o $(objdir)Executor.o\
	$(objdir)QueryPlan.o $(objdir)Statistics.o $(objdir)PlanTree.o $(objdir)PlanGenerator.o $(objdir)OrderedVector.o $(objdir)DPBTopKUtil.o
	$(CXX) $(CFLAGS) Database/Optimizer.cpp $(inc) $(inc_log) -o $(objdir)Optimizer.o $(openmp) ${ldl}

$(objdir)Txn_manager.o: Database/Txn_manager.cpp Database/Txn_manager.h $(objdir)Util.o $(objdir)Transaction.o $(objdir)Database.o
	$(CXX) $(CFLAGS) Database/Txn_manager.cpp $(inc) $(inc_log) -o $(objdir)Txn_manager.o $(openmp)

#objects in Database/ end


#objects in Query/ begin

$(objdir)IDList.o: Query/IDList.cpp Query/IDList.h
	$(CXX) $(CFLAGS) Query/IDList.cpp $(inc) $(inc_log) -o $(objdir)IDList.o $(openmp)

$(objdir)SPARQLquery.o: Query/SPARQLquery.cpp Query/SPARQLquery.h $(objdir)BasicQuery.o
	$(CXX) $(CFLAGS) Query/SPARQLquery.cpp $(inc) $(inc_log) -o $(objdir)SPARQLquery.o $(openmp)

$(objdir)BasicQuery.o: Query/BasicQuery.cpp Query/BasicQuery.h $(objdir)Signature.o
	$(CXX) $(CFLAGS) Query/BasicQuery.cpp $(inc) $(inc_log) -o $(objdir)BasicQuery.o $(openmp)

$(objdir)ResultSet.o: Query/ResultSet.cpp Query/ResultSet.h $(objdir)Stream.o
	$(CXX) $(CFLAGS) Query/ResultSet.cpp $(inc) $(inc_log) -o $(objdir)ResultSet.o $(openmp)

$(objdir)Varset.o: Query/Varset.cpp Query/Varset.h
	$(CXX) $(CFLAGS) Query/Varset.cpp $(inc) $(inc_log) -o $(objdir)Varset.o $(openmp)

$(objdir)QueryPlan.o: Query/QueryPlan.cpp Query/QueryPlan.h $(objdir)BasicQuery.o $(objdir)TableOperator.o $(objdir)ResultTrigger.o
	$(CXX) $(CFLAGS) Query/QueryPlan.cpp $(inc) $(inc_log) -o $(objdir)QueryPlan.o $(openmp)

$(objdir)QueryTree.o: Query/QueryTree.cpp Query/QueryTree.h $(objdir)Varset.o
	$(CXX) $(CFLAGS) Query/QueryTree.cpp $(inc) $(inc_log) -o $(objdir)QueryTree.o $(openmp)

$(objdir)TempResult.o: Query/TempResult.cpp Query/TempResult.h Query/RegexExpression.h $(objdir)Util.o \
	$(objdir)StringIndex.o $(objdir)QueryTree.o $(objdir)Varset.o $(objdir)EvalMultitypeValue.o
	$(CXX) $(CFLAGS) Query/TempResult.cpp $(inc) $(inc_log) -o $(objdir)TempResult.o $(openmp)

$(objdir)QueryCache.o: Query/QueryCache.cpp Query/QueryCache.h $(objdir)Util.o $(objdir)QueryTree.o \
	$(objdir)TempResult.o $(objdir)Varset.o
	$(CXX) $(CFLAGS) Query/QueryCache.cpp $(inc) $(inc_log) -o $(objdir)QueryCache.o $(openmp)

$(objdir)PathQueryHandler.o: Query/PathQueryHandler.cpp Query/PathQueryHandler.h $(objdir)CSR.o
	$(CXX) $(CFLAGS) Query/PathQueryHandler.cpp $(inc) $(inc_log) -o $(objdir)PathQueryHandler.o $(openmp) ${ldl}
	$(CXX) -std=c++11 -fPIC -shared Query/PathQueryHandler.cpp -o lib/libgpathqueryhandler.so lib/libgcsr.so

$(objdir)BGPQuery.o: Query/BGPQuery.cpp Query/BGPQuery.h $(objdir)BasicQuery.o  $(objdir)Util.o \
    $(objdir)Triple.o $(objdir)KVstore.o
	$(CXX) $(CFLAGS) Query/BGPQuery.cpp $(inc) $(inc_log) -o $(objdir)BGPQuery.o $(openmp)

#objects in Query/topk/ begin

$(objdir)Pool.o: Query/topk/DPB/Pool.cpp Query/topk/DPB/Pool.h $(objdir)Util.o
	$(CXX) $(CFLAGS) Query/topk/DPB/Pool.cpp $(inc) $(inc_log) -o $(objdir)Pool.o $(openmp)

$(objdir)DynamicTrie.o: Query/topk/DPB/DynamicTrie.cpp Query/topk/DPB/DynamicTrie.h $(objdir)Util.o $(objdir)Pool.o
	$(CXX) $(CFLAGS) Query/topk/DPB/DynamicTrie.cpp $(inc) $(inc_log) -o $(objdir)DynamicTrie.o $(openmp)

$(objdir)OrderedList.o: Query/topk/DPB/OrderedList.cpp Query/topk/DPB/OrderedList.h $(objdir)Util.o $(objdir)Pool.o \
	$(objdir)DynamicTrie.o
	$(CXX) $(CFLAGS) Query/topk/DPB/OrderedList.cpp $(inc) $(inc_log) -o $(objdir)OrderedList.o $(openmp)

$(objdir)TopKSearchPlan.o: Query/topk/TopKSearchPlan.cpp Query/topk/TopKSearchPlan.h $(objdir)Util.o \
	$(objdir)KVstore.o $(objdir)OrderedList.o $(objdir)SPARQLquery.o $(objdir)BasicQuery.o \
	$(objdir)Statistics.o $(objdir)QueryTree.o $(objdir)IDList.o \
	$(objdir)PlanGenerator.o $(objdir)TableOperator.o
	$(CXX) $(CFLAGS) Query/topk/TopKSearchPlan.cpp $(inc) $(inc_log) -o $(objdir)TopKSearchPlan.o $(openmp)

$(objdir)TopKUtil.o: Query/topk/TopKUtil.cpp Query/topk/TopKUtil.h $(objdir)Util.o $(objdir)KVstore.o \
	$(objdir)OrderedList.o $(objdir)SPARQLquery.o $(objdir)BasicQuery.o $(objdir)Statistics.o \
	$(objdir)QueryTree.o $(objdir)IDList.o $(objdir)TableOperator.o $(objdir)TopKSearchPlan.o
	$(CXX) $(CFLAGS) Query/topk/TopKUtil.cpp $(inc) $(inc_log) -o $(objdir)TopKUtil.o $(openmp)


$(objdir)DPBTopKUtil.o: Query/topk/DPBTopKUtil.cpp Query/topk/DPBTopKUtil.h $(objdir)TopKUtil.o
	$(CXX) $(CFLAGS) Query/topk/DPBTopKUtil.cpp $(inc) $(inc_log) -o $(objdir)DPBTopKUtil.o $(openmp)

#objects in Query/topk/ end


#no more using $(objdir)Database.o
$(objdir)GeneralEvaluation.o: Query/GeneralEvaluation.cpp Query/GeneralEvaluation.h Query/RegexExpression.h \
	$(objdir)VSTree.o $(objdir)KVstore.o $(objdir)StringIndex.o $(objdir)Strategy.o $(objdir)QueryParser.o \
	$(objdir)Triple.o $(objdir)Util.o $(objdir)EvalMultitypeValue.o $(objdir)SPARQLquery.o $(objdir)QueryTree.o $(objdir)Varset.o  $(objdir)Statistics.o \
	$(objdir)TempResult.o $(objdir)QueryCache.o $(objdir)ResultSet.o $(objdir)PathQueryHandler.o $(objdir)Optimizer.o
	$(CXX) $(CFLAGS) Query/GeneralEvaluation.cpp $(inc) $(inc_log) -o $(objdir)GeneralEvaluation.o $(openmp) ${ldl}

#objects in Query/ end


#objects in Signature/ begin

$(objdir)SigEntry.o: Signature/SigEntry.cpp Signature/SigEntry.h $(objdir)Signature.o
	$(CXX) $(CFLAGS) Signature/SigEntry.cpp $(inc) $(inc_log) -o $(objdir)SigEntry.o $(openmp)

$(objdir)Signature.o: Signature/Signature.cpp Signature/Signature.h
	$(CXX) $(CFLAGS) Signature/Signature.cpp $(inc) $(inc_log) -o $(objdir)Signature.o $(openmp)

#objects in Signature/ end


#objects in Util/ begin

$(objdir)Util.o:  Util/Util.cpp Util/Util.h
	$(CXX) $(CFLAGS) Util/Util.cpp $(inc_log) -o $(objdir)Util.o $(openmp)

$(objdir)WebUrl.o:  Util/WebUrl.cpp Util/WebUrl.h
	$(CXX) $(CFLAGS) Util/WebUrl.cpp -o $(objdir)WebUrl.o $(openmp)



$(objdir)INIParser.o:  Util/INIParser.cpp Util/INIParser.h
	$(CXX) $(CFLAGS) Util/INIParser.cpp -o $(objdir)INIParser.o $(openmp)

$(objdir)Slog.o:  Util/Slog.cpp Util/Slog.h $(lib_log)
	$(CXX) $(CFLAGS) Util/Slog.cpp $(inc_log) -o $(objdir)Slog.o $(openmp)

#$(objdir)grpc.srpc.o:   GRPC/grpc.srpc.h $(lib_workflow)
#	$(CXX) $(CFLAGS)  GRPC/grpc.srpc.h -o $(objdir)grpc.srpc.o $(openmp)

$(objdir)Stream.o:  Util/Stream.cpp Util/Stream.h $(objdir)Util.o $(objdir)Bstr.o
	$(CXX) $(CFLAGS) Util/Stream.cpp $(inc_log) -o $(objdir)Stream.o $(def64IO) $(openmp)

$(objdir)Bstr.o: Util/Bstr.cpp Util/Bstr.h $(objdir)Util.o
	$(CXX) $(CFLAGS)  Util/Bstr.cpp $(inc_log) -o $(objdir)Bstr.o $(openmp)

$(objdir)Triple.o: Util/Triple.cpp Util/Triple.h $(objdir)Util.o
	$(CXX) $(CFLAGS) Util/Triple.cpp $(inc_log) -o $(objdir)Triple.o $(openmp)

$(objdir)VList.o:  Util/VList.cpp Util/VList.h
	$(CXX) $(CFLAGS) Util/VList.cpp $(inc_log) -o $(objdir)VList.o $(openmp)

$(objdir)EvalMultitypeValue.o: Util/EvalMultitypeValue.cpp Util/EvalMultitypeValue.h
	$(CXX) $(CFLAGS) Util/EvalMultitypeValue.cpp $(inc_log) -o $(objdir)EvalMultitypeValue.o $(openmp)

$(objdir)Version.o: Util/Version.cpp Util/Version.h
	$(CXX) $(CFLAGS) Util/Version.cpp $(inc_log) -o $(objdir)Version.o $(openmp)

$(objdir)SpinLock.o: Util/SpinLock.h Util/SpinLock.cpp
	$(CXX) $(CFLAGS) Util/SpinLock.cpp $(inc_log) -o $(objdir)SpinLock.o $(openmp)

$(objdir)GraphLock.o: Util/GraphLock.h Util/GraphLock.cpp
	$(CXX) $(CFLAGS) Util/GraphLock.cpp $(inc_log) -o $(objdir)GraphLock.o $(openmp)

$(objdir)Transaction.o: Util/Transaction.cpp Util/Transaction.h $(objdir)Util.o $(objdir)IDTriple.o
	$(CXX) $(CFLAGS) Util/Transaction.cpp $(inc) $(inc_log) -o $(objdir)Transaction.o $(openmp)

$(objdir)IDTriple.o: Util/IDTriple.cpp Util/IDTriple.h
	$(CXX) $(CFLAGS) Util/IDTriple.cpp $(inc_log) -o $(objdir)IDTriple.o $(openmp)

$(objdir)Latch.o: Util/Latch.cpp Util/Latch.h
	$(CXX) $(CFLAGS) Util/Latch.cpp $(inc_log) -o $(objdir)Latch.o $(openmp)

$(objdir)IPWhiteList.o:  Util/IPWhiteList.cpp Util/IPWhiteList.h $(objdir)Util.o
	$(CXX) $(CFLAGS) Util/IPWhiteList.cpp $(inc_log) -o $(objdir)IPWhiteList.o $(def64IO) $(openmp)

$(objdir)IPBlackList.o:  Util/IPBlackList.cpp Util/IPBlackList.h $(objdir)Util.o
	$(CXX) $(CFLAGS) Util/IPBlackList.cpp $(inc_log) -o $(objdir)IPBlackList.o $(def64IO) $(openmp)

$(objdir)OrderedVector.o: Util/OrderedVector.cpp Util/OrderedVector.h
	$(CXX) $(CFLAGS) Util/OrderedVector.cpp $(inc_log) -o $(objdir)OrderedVector.o $(openmp)

#objects in util/ end


#objects in VSTree/ begin

$(objdir)VSTree.o: VSTree/VSTree.cpp VSTree/VSTree.h $(objdir)EntryBuffer.o $(objdir)LRUCache.o $(objdir)VNode.o
	$(CXX) $(CFLAGS) VSTree/VSTree.cpp $(inc) $(inc_log) -o $(objdir)VSTree.o $(def64IO) $(openmp)

$(objdir)EntryBuffer.o: VSTree/EntryBuffer.cpp VSTree/EntryBuffer.h Signature/SigEntry.h
	$(CXX) $(CFLAGS) VSTree/EntryBuffer.cpp $(inc) $(inc_log) -o $(objdir)EntryBuffer.o $(def64IO) $(openmp)

$(objdir)LRUCache.o: VSTree/LRUCache.cpp  VSTree/LRUCache.h VSTree/VNode.h
	$(CXX) $(CFLAGS) VSTree/LRUCache.cpp $(inc) $(inc_log) -o $(objdir)LRUCache.o $(def64IO) $(openmp)

$(objdir)VNode.o: VSTree/VNode.cpp VSTree/VNode.h
	$(CXX) $(CFLAGS) VSTree/VNode.cpp $(inc) $(inc_log) -o $(objdir)VNode.o $(def64IO) $(openmp)

#objects in VSTree/ end


#objects in StringIndex/ begin
$(objdir)StringIndex.o: StringIndex/StringIndex.cpp StringIndex/StringIndex.h $(objdir)KVstore.o $(objdir)Util.o
	$(CXX) $(CFLAGS) StringIndex/StringIndex.cpp $(inc) $(inc_log) -o $(objdir)StringIndex.o $(def64IO) $(openmp)
#objects in StringIndex/ end


#objects in Parser/ begin

$(objdir)SPARQLParser.o: Parser/SPARQL/SPARQLParser.cpp Parser/SPARQL/SPARQLParser.h
	$(CXX)  $(CFLAGS) Parser/SPARQL/SPARQLParser.cpp $(inc) -o $(objdir)SPARQLParser.o $(openmp)

$(objdir)SPARQLLexer.o: Parser/SPARQL/SPARQLLexer.cpp Parser/SPARQL/SPARQLLexer.h
	$(CXX)  $(CFLAGS) Parser/SPARQL/SPARQLLexer.cpp $(inc) -o $(objdir)SPARQLLexer.o $(openmp)

$(objdir)TurtleParser.o: Parser/TurtleParser.cpp Parser/TurtleParser.h Parser/Type.h
	$(CXX)  $(CFLAGS) Parser/TurtleParser.cpp $(inc) $(inc_log) -o $(objdir)TurtleParser.o $(openmp)

$(objdir)RDFParser.o: Parser/RDFParser.cpp Parser/RDFParser.h $(objdir)TurtleParser.o $(objdir)Triple.o
	$(CXX)  $(CFLAGS) Parser/RDFParser.cpp $(inc) $(inc_log) -o $(objdir)RDFParser.o $(openmp)

$(objdir)QueryParser.o: Parser/QueryParser.cpp Parser/QueryParser.h $(objdir)SPARQLParser.o $(objdir)SPARQLLexer.o $(objdir)QueryTree.o
	$(CXX) $(CFLAGS) Parser/QueryParser.cpp $(inc) $(inc_log) -o $(objdir)QueryParser.o $(openmp)

#objects in Parser/ end

#objects in Trie/ begin

$(objdir)TrieNode.o: Trie/TrieNode.cpp Trie/TrieNode.h
	$(CXX) $(CFLAGS) Trie/TrieNode.cpp -o $(objdir)TrieNode.o

$(objdir)Trie.o: Trie/Trie.cpp Trie/Trie.h $(objdir)TrieNode.o $(objdir)Triple.o $(objdir)RDFParser.o
	$(CXX) $(CFLAGS) Trie/Trie.cpp $(inc) $(inc_log) -o $(objdir)Trie.o

#objects in Server/ begin

$(objdir)Operation.o: Server/Operation.cpp Server/Operation.h
	$(CXX) $(CFLAGS) Server/Operation.cpp $(inc) $(inc_log) -o $(objdir)Operation.o $(openmp)

$(objdir)Socket.o: Server/Socket.cpp Server/Socket.h
	$(CXX) $(CFLAGS) Server/Socket.cpp $(inc) $(inc_log) -o $(objdir)Socket.o $(openmp)

$(objdir)Server.o: Server/Server.cpp Server/Server.h $(objdir)Socket.o $(objdir)Database.o $(objdir)Operation.o
	$(CXX) $(CFLAGS) Server/Server.cpp $(inc) $(inc_log) -o $(objdir)Server.o $(openmp)

# $(objdir)client_http.o: Server/client_http.hpp
# 	$(CXX) $(CFLAGS) Server/client_http.hpp $(inc) -o $(objdir)client_http.o

# $(objdir)server_http.o: Server/server_http.hpp
# 	$(CXX) $(CFLAGS) Server/server_http.hpp $(inc) -o $(objdir)server_http.o

#objects in Server/ end

#objects in GRPC/ begin

$(objdir)APIUtil.o: GRPC/APIUtil.cpp GRPC/APIUtil.h Database/Database.h Database/Txn_manager.h Util/Util.h $(lib_antlr)
	$(CXX) $(CFLAGS) GRPC/APIUtil.cpp $(inc) $(inc_log) -o $(objdir)APIUtil.o -DUSE_BOOST_REGEX $(def64IO) $(openmp)

$(objdir)grpc_status_code.o: GRPC/grpc_status_code.cpp GRPC/grpc_status_code.h $(lib_antlr) $(lib_rpc)
	$(CXX) $(CFLAGS) GRPC/grpc_status_code.cpp $(inc) $(inc_rpc) -o $(objdir)grpc_status_code.o $(def64IO) $(openmp)

$(objdir)grpc_content.o: GRPC/grpc_content.cpp GRPC/grpc_content.h $(lib_antlr) $(lib_rpc)
	$(CXX) $(CFLAGS) GRPC/grpc_content.cpp $(inc) $(inc_rpc) -o $(objdir)grpc_content.o $(def64IO) $(openmp)

$(objdir)grpc_message.o: GRPC/grpc_message.cpp GRPC/grpc_message.h GRPC/grpc_noncopyable.h $(objdir)grpc_content.o $(lib_antlr) $(lib_rpc)
	$(CXX) $(CFLAGS) GRPC/grpc_message.cpp $(inc) $(inc_rpc) -o $(objdir)grpc_message.o $(def64IO) $(openmp)

$(objdir)grpc_server_task.o: GRPC/grpc_server_task.cpp GRPC/grpc_server_task.h $(objdir)grpc_message.o GRPC/grpc_noncopyable.h $(lib_antlr) $(lib_rpc)
	$(CXX) $(CFLAGS) GRPC/grpc_server_task.cpp $(inc) $(inc_rpc) -o $(objdir)grpc_server_task.o $(def64IO) $(openmp)

$(objdir)grpc_routetable.o: GRPC/grpc_routetable.cpp GRPC/grpc_routetable.h GRPC/grpc_request_handler.h GRPC/grpc_noncopyable.h GRPC/grpc_stringpiece.h $(lib_antlr) $(lib_rpc)
	$(CXX) $(CFLAGS) GRPC/grpc_routetable.cpp $(inc) $(inc_rpc) -o $(objdir)grpc_routetable.o $(def64IO) $(openmp)

$(objdir)grpc_router.o: GRPC/grpc_router.cpp GRPC/grpc_router.h $(objdir)grpc_routetable.o GRPC/grpc_noncopyable.h $(objdir)grpc_server_task.o $(lib_antlr) $(lib_rpc)
	$(CXX) $(CFLAGS) GRPC/grpc_router.cpp $(inc) $(inc_rpc) -o $(objdir)grpc_router.o $(def64IO) $(openmp)

$(objdir)grpc_server.o: GRPC/grpc_server.cpp GRPC/grpc_server.h $(objdir)grpc_message.o $(objdir)grpc_router.o $(lib_antlr) $(lib_rpc)
	$(CXX) $(CFLAGS) GRPC/grpc_server.cpp $(inc) $(inc_rpc) -o $(objdir)grpc_server.o $(def64IO) $(openmp)

#objects in GRPC/ end

# your gcc g++ v5.4 path
# in ./bashrc CXX should be gcc, otherwise, make pre2 will error
# see https://blog.csdn.net/weixin_34268610/article/details/89085852
#pre1:export CXX=/usr/local/gcc-5.4.0/bin/gcc
#pre1:export CXX=/usr/local/gcc-5.4.0/bin/g++
#pre1:
#	cd tools; tar -xvf log4cplus-1.2.0.tar;cd log4cplus-1.2.0;./configure;make;sudo make install;

pre:
	rm -rf tools/rapidjson/
	rm -rf tools/antlr4-cpp-runtime-4/
	rm -rf tools/workflow
	rm -rf tools/log4cplus
	rm -rf lib/libantlr4-runtime.a lib/libworkflow.a lib/liblog4cplus.a
	cd tools; tar -xzvf rapidjson.tar.gz;
	cd tools; tar -xzvf antlr4-cpp-runtime-4.tar.gz;
	cd tools; tar -xzvf workflow-0.10.3.tar.gz;
	cd tools; tar -xzvf log4cplus-2.0.8.tar.gz;
	cd tools/antlr4-cpp-runtime-4/; cmake .; make; cp dist/libantlr4-runtime.a ../../lib/;
	cd tools/workflow; make; cp _lib/libworkflow.a ../../lib/;
	cd tools/log4cplus; ./configure --enable-static; make; cp .libs/liblog4cplus.a ../../lib/;
$(api_cpp):
	$(MAKE) -C api/http/cpp/src

$(api_socket):
	$(MAKE) -C api/socket/cpp/src


.PHONY: clean dist tarball api_example gtest sumlines contribution test

test: $(TARGET)
	@echo "basic build/query/add/sub/drop test......"
	@bash scripts/basic_test.sh
	@echo "repeatedly insertion/deletion test......"
	@scripts/update_test > /dev/null
	@echo "parser test......"
	@bash scripts/parser_test.sh

clean:
	#rm -rf lib/libantlr4-runtime.a
	$(MAKE) -C api/socket/cpp/src clean
	$(MAKE) -C api/socket/cpp/example clean
	$(MAKE) -C api/http/cpp/src clean
	$(MAKE) -C api/http/cpp/example clean
	$(MAKE) -C api/http/java/src clean
	$(MAKE) -C api/http/java/example clean
	#$(MAKE) -C KVstore clean
	rm -rf $(exedir)g* $(objdir)*.o $(exedir).gserver* $(exedir)shutdown $(exedir)rollback
	rm -rf bin/*.class
	rm -rf $(testdir)update_test $(testdir)dataset_test $(testdir)transaction_test $(testdir)run_transaction $(testdir)workload $(testdir)debug_test
	#rm -rf .project .cproject .settings   just for eclipse
	rm -rf logs/*.log
	rm -rf *.out   # gmon.out for gprof with -pg
	rm -rf lib/libgcsr.so lib/libgpathqueryhandler.so


dist: clean
	rm -rf *.nt *.n3 .debug/*.log .tmp/*.dat *.txt *.db
	rm -rf lib/libantlr4-runtime.a
	rm -rf cscope* .cproject .settings tags
	rm -rf *.info
	rm -rf backups/*.db

tarball:
	tar -czvf gstore.tar.gz api backups bin lib tools .debug .tmp .objs scripts docs data logs \
		Main Database KVstore Util Query Signature VSTree Parser Server README.md init.conf conf.ini StringIndex COVERAGE \
		Dockerfile LICENSE makefile Trie

APIexample: $(api_cpp) $(api_socket) 
	$(MAKE) -C api/http/cpp/example
	$(MAKE) -C api/socket/cpp/example

gtest: $(objdir)gtest.o $(objfile)
	$(CXX) $(EXEFLAG) -o $(exedir)gtest $(objdir)gtest.o $(objfile) lib/libantlr4-runtime.a $(library) $(openmp)

$(objdir)gtest.o: scripts/gtest.cpp
	$(CXX) $(CFLAGS) scripts/gtest.cpp $(inc) -o $(objdir)gtest.o $(openmp)
	
$(exedir)gadd: $(objdir)gadd.o $(objfile)
	$(CXX) $(EXEFLAG) -o $(exedir)gadd $(objdir)gadd.o $(objfile) lib/libantlr4-runtime.a $(library) $(openmp) ${ldl}

$(objdir)gadd.o: Main/gadd.cpp
	$(CXX) $(CFLAGS) Main/gadd.cpp $(inc) $(inc_log) -o $(objdir)gadd.o $(openmp)

#$(objdir)HttpConnector: $(objdir)HttpConnector.o $(objfile)
	#$(CXX) $(CFLAGS) -o $(exedir)HttpConnector $(objdir)HttpConnector.o $(objfile) lib/libantlr4-runtime.a $(library) $(inc) -DUSE_BOOST_REGEX

#$(objdir)HttpConnector.o: Main/HttpConnector.cpp
	#$(CXX) $(CFLAGS) Main/HttpConnector.cpp $(inc) -o $(objdir)HttpConnector.o $(library) -DUSE_BOOST_REGEX

$(exedir)gsub: $(objdir)gsub.o $(objfile)
	$(CXX) $(EXEFLAG) -o $(exedir)gsub $(objdir)gsub.o $(objfile) lib/libantlr4-runtime.a $(library) $(openmp) ${ldl}

$(objdir)gsub.o: Main/gsub.cpp
	$(CXX) $(CFLAGS) Main/gsub.cpp $(inc) $(inc_log) -o $(objdir)gsub.o $(openmp)

sumlines:
	@bash scripts/sumline.sh

tag:
	ctags -R

idx:
	find `realpath .` -name "*.h" -o -name "*.c" -o -name "*.cpp" > cscope.files
	cscope -bkq #-i cscope.files

cover:
	bash scripts/cover.sh

fulltest:
	#NOTICE:compile gstore with -O2 only
	#setup new virtuoso and configure it
	cp scripts/full_test.sh ~
	cd ~
	bash full_test.sh

#test the efficience of kvstore, insert/delete/search, use dbpedia170M by default
test-kvstore:
	# test/kvstore_test.cpp
	echo "TODO"

# https://segmentfault.com/a/1190000008542123
contribution:
	bash scripts/contribution.sh

