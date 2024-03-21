include_directories( ${INCLUDE_3RD_PATH} )
link_directories( ${LIBRARY_LINK_PATH} )
aux_source_directory( . SRC_LIST )
add_library( stringindex ${SRC_LIST} )

target_link_libraries( stringindex
   util
   antlr4-runtime
)