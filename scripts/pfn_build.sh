#/bin/bash

rm -rf ./pfn/include/
rm -rf ./pfn/lib/liblog4cplus.a
mkdir -p ./pfn/include/
mkdir -p ./pfn/include/Database/
mkdir -p ./pfn/include/KVstore/
mkdir -p ./pfn/include/KVstore/ISArray/
mkdir -p ./pfn/include/KVstore/IVArray/
mkdir -p ./pfn/include/KVstore/SITree/
mkdir -p ./pfn/include/KVstore/SITree/heap/
mkdir -p ./pfn/include/KVstore/SITree/node/
mkdir -p ./pfn/include/KVstore/SITree/storage/
mkdir -p ./pfn/include/Parser/
mkdir -p ./pfn/include/Query/
mkdir -p ./pfn/include/Query/Algorithm/
mkdir -p ./pfn/include/Trie/
mkdir -p ./pfn/include/Util/
mkdir -p ./pfn/include/Pfn/
mkdir -p ./pfn/include/nlohmann/

# log4cplus
cp -f ./lib/liblog4cplus.a ./pfn/lib/
cp -r ./include/log4cplus ./pfn/include/
# nlohmann
cp -f ./include/nlohmann/json.hpp ./pfn/include/nlohmann/
# database
cp -f ./src/Database/CSR.h ./pfn/include/Database/
# kvstore
cp -f ./src/KVstore/KVstore.h ./pfn/include/KVstore/
cp -f ./src/KVstore/EntryBlockList.h ./pfn/include/KVstore/
cp -f ./src/KVstore/Tree.h ./pfn/include/KVstore/
cp -f ./src/KVstore/ISArray/ISArray.h ./pfn/include/KVstore/ISArray/
cp -f ./src/KVstore/ISArray/ISBlockManager.h ./pfn/include/KVstore/ISArray/
cp -f ./src/KVstore/ISArray/ISEntry.h ./pfn/include/KVstore/ISArray/
cp -f ./src/KVstore/IVArray/IVArray.h ./pfn/include/KVstore/IVArray/
cp -f ./src/KVstore/IVArray/IVBlockManager.h ./pfn/include/KVstore/IVArray/
cp -f ./src/KVstore/IVArray/IVEntry.h ./pfn/include/KVstore/IVArray/
cp -f ./src/KVstore/SITree/SITree.h ./pfn/include/KVstore/SITree/
cp -f ./src/KVstore/SITree/SITree.h ./pfn/include/KVstore/SITree/
cp -f ./src/KVstore/SITree/heap/SIHeap.h ./pfn/include/KVstore/SITree/heap/
cp -f ./src/KVstore/SITree/node/SIIntlNode.h ./pfn/include/KVstore/SITree/node/
cp -f ./src/KVstore/SITree/node/SILeafNode.h ./pfn/include/KVstore/SITree/node/
cp -f ./src/KVstore/SITree/node/SINode.h ./pfn/include/KVstore/SITree/node/
cp -f ./src/KVstore/SITree/storage/SIStorage.h ./pfn/include/KVstore/SITree/storage/
# Parser
cp -f ./src/Parser/RDFParser.h ./pfn/include/Parser/
cp -f ./src/Parser/TurtleParser.h ./pfn/include/Parser/
cp -f ./src/Parser/Type.h ./pfn/include/Parser/
# Trie
cp -f ./src/Trie/Trie.h ./pfn/include/Trie/
cp -f ./src/Trie/TrieNode.h ./pfn/include/Trie/
# Query
cp -f ./src/Query/Algorithm/CSRQueryHandler.h ./pfn/include/Query/Algorithm/
# Pfn
cp -f ./src/Pfn/GAnalysis.h ./pfn/include/Pfn/
# Util
cp -f ./src/Util/Bstr.h ./pfn/include/Util/
cp -f ./src/Util/ClassForVlistCache.h ./pfn/include/Util/
cp -f ./src/Util/EvalMultitypeValue.h ./pfn/include/Util/
cp -f ./src/Util/FileUtil.h ./pfn/include/Util/
cp -f ./src/Util/GlobalTypedef.h ./pfn/include/Util/
cp -f ./src/Util/GraphLock.h ./pfn/include/Util/
cp -f ./src/Util/IDTriple.h ./pfn/include/Util/
cp -f ./src/Util/IdUtil.h ./pfn/include/Util/
cp -f ./src/Util/INIParser.h ./pfn/include/Util/
cp -f ./src/Util/JsonUtil.h ./pfn/include/Util/
cp -f ./src/Util/Latch.h ./pfn/include/Util/
cp -f ./src/Util/MD5.h ./pfn/include/Util/
cp -f ./src/Util/NodeUtil.h ./pfn/include/Util/
cp -f ./src/Util/PrettyPrint.h ./pfn/include/Util/
cp -f ./src/Util/ResourceUtil.h ./pfn/include/Util/
cp -f ./src/Util/SpinLock.h ./pfn/include/Util/
cp -f ./src/Util/Stream.h ./pfn/include/Util/
cp -f ./src/Util/StringUtil.h ./pfn/include/Util/
cp -f ./src/Util/ThreadUtil.h ./pfn/include/Util/
cp -f ./src/Util/TimeUtil.h ./pfn/include/Util/
cp -f ./src/Util/Transaction.h ./pfn/include/Util/
cp -f ./src/Util/Triple.h ./pfn/include/Util/
cp -f ./src/Util/Util.h ./pfn/include/Util/
cp -f ./src/Util/Version.h ./pfn/include/Util/
cp -f ./src/Util/VList.h ./pfn/include/Util/


