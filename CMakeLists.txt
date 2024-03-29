project( gstore2.0 )

cmake_minimum_required( VERSION 3.17 )
message( STATUS "this is source dir " ${CMAKE_SOURCE_DIR} )
message( STATUS "this is current binary dir " ${CMAKE_BINARY_DIR} )

# default output path bin and lib
set( EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/../bin" )
set( LIBRARY_OUTPUT_PATH  "${PROJECT_BINARY_DIR}/lib" )
set( GSTORE_ROOT_DIR ${CMAKE_SOURCE_DIR} )
# build generate bin, make pre generate include and lib
set( GSTORE_3RDINCLUDE_DIR ${GSTORE_ROOT_DIR}/include )
set( GSTORE_3RDLIB_DIR ${GSTORE_ROOT_DIR}/lib )
set( GSTORE_PFNLIB_DIR ${GSTORE_ROOT_DIR}/pfn/lib )
# scripts test
set( GSTORE_TEST_DIR ${GSTORE_ROOT_DIR}/scripts/test )

set( INCLUDE_3RD_PATH
        ${GSTORE_ROOT_DIR}/src
        ${GSTORE_3RDINCLUDE_DIR}
        ${GSTORE_3RDINCLUDE_DIR}/rapidjson
        ${GSTORE_3RDINCLUDE_DIR}/antlr4
        ${GSTORE_3RDINCLUDE_DIR}/workflow
        ${GSTORE_3RDINCLUDE_DIR}/log4cplus
        ${GSTORE_3RDINCLUDE_DIR}/indicators
)

add_subdirectory( src )
add_subdirectory( ${GSTORE_TEST_DIR} )

# init system.db
add_custom_command( OUTPUT INIT
    COMMAND echo "Compilation ends successfully!"
    COMMAND bash scripts/init.sh
    WORKING_DIRECTORY ${GSTORE_ROOT_DIR}/
)
add_custom_target( "init" DEPENDS INIT )

# make pre
add_custom_target( pre
    COMMAND [ -d ${GSTORE_3RDINCLUDE_DIR} ] && [ -d ${GSTORE_3RDLIB_DIR} ] || bash init.sh
    WORKING_DIRECTORY ${GSTORE_ROOT_DIR}/3rdparty/
)

#make clean_pre
add_custom_command( OUTPUT CLEAN
    COMMAND bash clean.sh
    COMMAND rm -rf ../logs/*.log
    WORKING_DIRECTORY ${GSTORE_ROOT_DIR}/3rdparty/
)
add_custom_target( "clean_pre" DEPENDS CLEAN )

#make test
add_custom_command( OUTPUT TEST
    COMMAND echo "basic build/query/add/sub/drop test......"
	COMMAND bash scripts/test/basic_test.sh
	COMMAND echo "parser test......"
	COMMAND bash scripts/test/parser_test.sh
    COMMAND echo "repeatedly insertion/deletion test num:20......"
    COMMAND scripts/test/update_test 20
    COMMAND echo "repeatedly insertion/deletion test end"
    WORKING_DIRECTORY ${GSTORE_ROOT_DIR}
)
add_custom_target( "test" DEPENDS TEST )

#make dist:clean_pre
add_custom_command( OUTPUT DIST
    COMMAND rm -rf *.nt *.n3 .debug/*.log .tmp/*.dat *.txt *.db
    COMMAND rm -rf cscope* .cproject .settings tags
    COMMAND rm -rf *.info
    COMMAND backups/*.db
    WORKING_DIRECTORY ${GSTORE_ROOT_DIR}
)
add_custom_target( "dist" DEPENDS CLEAN DIST )

#make tarball
add_custom_command( OUTPUT TARBALL
    COMMAND tar -czvf gstore.tar.gz backups .tmp api conf 3rdparty scripts data logs pfn
    src CMakeLists.txt README.md LICENSE
    WORKING_DIRECTORY ${GSTORE_ROOT_DIR}
)
add_custom_target( "tarball" DEPENDS TARBALL )

#make APIexample
# add_custom_command( OUTPUT APIEXAMPLE
#     # COMMAND 
#     WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
# )
# add_custom_target( "APIexample" DEPENDS APIEXAMPLE )

#make gtest
# add_custom_command( OUTPUT GTEST
#     # COMMAND 
#     WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
# )
# add_custom_target( "gtest" DEPENDS GTEST )

#make sumlines
add_custom_command( OUTPUT SUMLINES
    COMMAND bash scripts/sumline.sh
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
add_custom_target( "sumlines" DEPENDS SUMLINES )

#make tag
add_custom_command( OUTPUT TAG
    COMMAND ctags -R
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
add_custom_target( "tag" DEPENDS TAG )

#make idx
add_custom_command( OUTPUT IDX
    COMMAND find `realpath .` -name "*.h" -o -name "*.c" -o -name "*.cpp" > cscope.files
    COMMAND cscope -bkq #-i cscope.files
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
add_custom_target( "idx" DEPENDS IDX )

#make cover
add_custom_command( OUTPUT COVER
    COMMAND bash scripts/cover.sh
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
add_custom_target( "cover" DEPENDS COVER )

#make fulltest
add_custom_command( OUTPUT FULLTEST
    #NOTICE:compile gstore with -O2 only
	#setup new virtuoso and configure it
    COMMAND cp ${PROJECT_SOURCE_DIR}/scripts/full_test.sh ./
    COMMAND bash full_test.sh
    WORKING_DIRECTORY ~
)
add_custom_target( "fulltest" DEPENDS FULLTEST )

#make test-kvstore:test the efficience of kvstore, insert/delete/search, use dbpedia170M by default
add_custom_command( OUTPUT TEST-KVSTORE
    # test/kvstore_test.cpp
	COMMAND echo "TODO"
)
add_custom_target( "test-kvstore" DEPENDS TEST-KVSTORE )

#make contribution
add_custom_command( OUTPUT CONTRIBUTION
    # https://segmentfault.com/a/1190000008542123
	COMMAND bash scripts/contribution.sh
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
add_custom_target( "contribution" DEPENDS CONTRIBUTION )