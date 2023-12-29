add_library(gstore_isarray OBJECT
        ISArray/ISArray.cpp
        ISArray/ISBlockManager.cpp
        ISArray/ISEntry.cpp
)

add_library(gstore_ivarray OBJECT
        IVArray/IVArray.cpp
        IVArray/IVBlockManager.cpp
        IVArray/IVEntry.cpp
)

add_library(gstore_sitree OBJECT
        SITree/SITree.cpp
        SITree/storage/SIStorage.cpp
        SITree/heap/SIHeap.cpp
        SITree/node/SIIntlNode.cpp
        SITree/node/SILeafNode.cpp
        SITree/node/SINode.cpp
)

add_library(gstore_kvstore OBJECT
        KVstore.cpp
        SITree/SITree.cpp
        SITree/storage/SIStorage.cpp
        SITree/heap/SIHeap.cpp
        SITree/node/SIIntlNode.cpp
        SITree/node/SILeafNode.cpp
        SITree/node/SINode.cpp
        ISArray/ISArray.cpp
        ISArray/ISBlockManager.cpp
        ISArray/ISEntry.cpp
        IVArray/IVArray.cpp
        IVArray/IVBlockManager.cpp
        IVArray/IVEntry.cpp
)