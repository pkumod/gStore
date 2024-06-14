if ( CMAKE_BUILD_TYPE STREQUAL Static )
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
)
set( BIN_LIST
        gadd
        gbackup
        gbuild
        gconsole
        gdrop
        gexport
        ghttp
        ginit
        gmonitor          
        gquery      
        grestore     
        grpc         
        gserver     
        gserver_backup_scheduler     
        gshow     
        gsub      
)
set( MAIN_LIST 
        gadd.cpp  
        gbackup.cpp 
        gbuild.cpp   
        gconsole.cpp 
        gdrop.cpp   
        gexport.cpp                  
        ghttp.cpp 
        ginit.cpp 
        gmonitor.cpp 
        gquery.cpp  
        grestore.cpp 
        grpc.cpp     
        gserver.cpp 
        gserver_backup_scheduler.cpp 
        gshow.cpp
        gsub.cpp 
)
include_directories( ${INCLUDE_3RD_PATH} )
link_directories( 
        ${GSTORE_3RDLIB_DIR}
        ${GSTORE_PFNLIB_DIR}
)
add_library( gcsr SHARED ../Database/CSR.cpp )
aux_source_directory( ../Query/Algorithm ALGORITHM_LIST )
add_library( gpathqueryhandler SHARED ${ALGORITHM_LIST} )
target_link_libraries( gpathqueryhandler gcsr )
set_target_properties( gcsr gpathqueryhandler PROPERTIES LIBRARY_OUTPUT_DIRECTORY  ${GSTORE_PFNLIB_DIR} )

add_executable( shutdown shutdown.cpp )
target_link_libraries( shutdown ${LIB_GSTROE_DEP} httputil ${LIB_3RD_DEP} ${LIB_SYSTEM_DEP} )
add_executable( rollback rollback.cpp )
target_link_libraries( rollback ${LIB_GSTROE_DEP} httputil ${LIB_3RD_DEP} ${LIB_SYSTEM_DEP} )
add_executable( gpara gpara.cpp )
target_link_libraries( gpara ${LIB_GSTROE_DEP} httputil ${LIB_3RD_DEP} ${LIB_SYSTEM_DEP} )

foreach( bin_name main_name IN ZIP_LISTS BIN_LIST MAIN_LIST )
   message( STATUS "bin_name = ${bin_name} main_name = ${main_name}" )
   add_executable( ${bin_name} ${main_name} )
   target_link_libraries( ${bin_name} ${LIB_GSTROE_DEP} ${LIB_3RD_DEP} ${LIB_SYSTEM_DEP} )
endforeach()
