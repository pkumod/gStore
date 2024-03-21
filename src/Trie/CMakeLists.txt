include_directories( ${INCLUDE_3RD_PATH} )
aux_source_directory( . SRC_LIST )
add_library( trie ${SRC_LIST} )