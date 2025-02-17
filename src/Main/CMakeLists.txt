# alter the cmakelist
if ( CMAKE_BUILD_TYPE STREQUAL Static OR CMAKE_BUILD_TYPE STREQUAL Cpack )
        set( LIB_SYSTEM_DEP
                boost_thread
                boost_system
                boost_regex
                readline
                ncurses
                tinfo
                curl
                ssl
                crypto
                pthread
                jemalloc
                dl
                gcov
        )
elseif(CMAKE_BUILD_TYPE STREQUAL StaticGcc OR CMAKE_BUILD_TYPE STREQUAL CpackGcc )
        set( LIB_SYSTEM_DEP
                boost_thread.a
                boost_system.a
                boost_regex.a
                readline.a
                ncurses.a
                tinfo.a
                curl.a
                ssl.a
                crypto.a
                pthread
                jemalloc.a
                dl
                gcov.a
        )
else ()
        set( LIB_SYSTEM_DEP
                boost_thread
                boost_system
                boost_regex
                readline
                curl
                ssl
                crypto
                pthread
                jemalloc
                dl
                gcov
        )
endif()

set( LIB_3RD_DEP
        workflow
        antlr4-runtime
        log4cplus
        z
        minizip
)
set( LIB_GSTROE_DEP 
        api
        database
        rpc
        kvstore
        parser
        query
        server
        stringindex
        util
        gcov
        reason
        pfn
        cluster
        httputil
)
set( BIN_LIST
        gconsole 
        gserver           
)
set( MAIN_LIST 
        gconsole.cpp 
        grpc.cpp     
)
include_directories( ${INCLUDE_3RD_PATH} )
link_directories( 
        ${GSTORE_3RDLIB_DIR}
        ${GSTORE_PFNLIB_DIR}
)

# ganalysis
aux_source_directory( ../KVstore/ KVSTORE_LIST )
aux_source_directory( ../KVstore/ISArray KVSTORE_LIST )
aux_source_directory( ../KVstore/IVArray KVSTORE_LIST )
aux_source_directory( ../KVstore/SITree KVSTORE_LIST )
aux_source_directory( ../KVstore/SITree/heap KVSTORE_LIST )
aux_source_directory( ../KVstore/SITree/node KVSTORE_LIST )
aux_source_directory( ../KVstore/SITree/storage KVSTORE_LIST )
add_library( ganalysis SHARED 
        ${KVSTORE_LIST}
        ../Util/Bstr.cpp
        ../Util/VList.cpp
        ../Util/Stream.cpp
        ../Util/EvalMultitypeValue.cpp
        ../Util/TimeUtil.cpp
        ../Util/ResourceUtil.cpp
        ../Util/StringUtil.cpp
        ../Util/SpinLock.cpp
        ../Util/INIParser.cpp
        ../Util/FileUtil.cpp
        ../Util/Latch.cpp
        ../Util/Version.cpp
        ../Util/Transaction.cpp
        ../Util/IDTriple.cpp
        ../Util/GraphLock.cpp
        ../Util/Latch.cpp
        ../Trie/Trie.cpp
        ../Trie/TrieNode.cpp
        ../Parser/RDFParser.cpp
        ../Parser/TurtleParser.cpp
        ../Pfn/GAnalysis.cpp
        ../Database/CSR.cpp
        ../Query/Algorithm/CSRQueryHandler.cpp )

set_target_properties( ganalysis PROPERTIES LINK_FLAGS "-Wl,-rpath,./" LIBRARY_OUTPUT_DIRECTORY  ${GSTORE_PFNLIB_DIR} )

foreach( bin_name main_name IN ZIP_LISTS BIN_LIST MAIN_LIST )
   message( STATUS "bin_name = ${bin_name} main_name = ${main_name}" )
   add_executable( ${bin_name} ${main_name} )
   if ( CMAKE_BUILD_TYPE STREQUAL Static OR CMAKE_BUILD_TYPE STREQUAL Cpack )
        target_link_libraries( ${bin_name} ${LIB_GSTROE_DEP} ${LIB_3RD_DEP} ${LIB_SYSTEM_DEP} )
   else ()
        target_link_libraries( ${bin_name} ${LIB_GSTROE_DEP} ganalysis ${LIB_3RD_DEP} ${LIB_SYSTEM_DEP} )
   endif()
endforeach()
