include_directories( ${INCLUDE_3RD_PATH} )
aux_source_directory( . SRC_LIST )
add_library( database ${SRC_LIST} )

target_link_libraries( database
   kvstore
   parser
   query
   trie
   stringindex
)