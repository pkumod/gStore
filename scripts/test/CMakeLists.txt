set( LIB_SYSTEM_DEP
        pthread
)
set( LIB_3RD_DEP
        antlr4-runtime
        log4cplus
)
set( LIB_GSTROE_DEP 
        database
        server
        util
)

include_directories( ${INCLUDE_3RD_PATH} )
link_directories( ${GSTORE_3RDLIB_DIR} )
add_executable( update_test update_test.cpp )
add_executable( gtest gtest.cpp )
set_target_properties( update_test gtest PROPERTIES RUNTIME_OUTPUT_DIRECTORY  ${GSTORE_TEST_DIR} )
target_link_libraries( update_test ${LIB_SYSTEM_DEP} ${LIB_GSTROE_DEP} ${LIB_3RD_DEP} )
target_link_libraries( gtest ${LIB_SYSTEM_DEP} ${LIB_GSTROE_DEP} ${LIB_3RD_DEP} )