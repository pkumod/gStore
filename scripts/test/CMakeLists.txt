
project( gstore2.0 )

cmake_minimum_required( VERSION 3.17 )
message( STATUS "this is source dir " ${CMAKE_SOURCE_DIR} )
message( STATUS "this is current binary dir " ${CMAKE_BINARY_DIR} )

# default output path bin and lib
set( EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/../bin" )
set( LIBRARY_OUTPUT_PATH  "${PROJECT_BINARY_DIR}/../lib" )
set( GSTORE_ROOT_DIR ${CMAKE_SOURCE_DIR}/../.. )
message( STATUS "this is current binary dir " ${GSTORE_ROOT_DIR} )
# build generate bin, make pre generate include and lib
set( GSTORE_LIB_DIR ${GSTORE_ROOT_DIR}/lib )
set( GSTORE_GSTORELIB_DIR ${CMAKE_SOURCE_DIR}/lib )
#3rdparty include
set( GSTORE_3RDPARTY_DIR ${GSTORE_ROOT_DIR}/include )
message( STATUS "this is current binary dir " ${GSTORE_3RDPARTY_DIR} )
set( INCLUDE_3RD_PATH
        ${GSTORE_3RDPARTY_DIR}
        ${GSTORE_3RDPARTY_DIR}/rapidjson
        ${GSTORE_3RDPARTY_DIR}/antlr4
        ${GSTORE_3RDPARTY_DIR}/workflow
        ${GSTORE_3RDPARTY_DIR}/log4cplus
        ${GSTORE_3RDPARTY_DIR}/indicators
)
set( LIB_SYSTEM_DEP
        boost_thread
        boost_system
        boost_regex
        readline
        curl
        ssl
        crypto
        pthread
        dl
)
set( LIB_3RD_DEP
        workflow
        antlr4-runtime
        log4cplus
        z
        minizip
)
set( LIB_GSTROE_DEP 
        database
        kvstore
        parser
        kvstore
        query
        trie
        stringindex
        server
        util
)

include_directories( ${INCLUDE_3RD_PATH} )
link_directories( ${GSTORE_LIB_DIR} )
link_directories( ${GSTORE_GSTORELIB_DIR} )
add_executable( update_test update_test.cpp )
target_link_libraries( update_test ${LIB_SYSTEM_DEP} ${LIB_GSTROE_DEP} ${LIB_3RD_DEP} )
add_executable( gtest gtest.cpp )
target_link_libraries( gtest ${LIB_SYSTEM_DEP} ${LIB_GSTROE_DEP} ${LIB_3RD_DEP} )