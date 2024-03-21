include_directories( ${INCLUDE_3RD_PATH} )
aux_source_directory( . SRC_LIST )
aux_source_directory( ./SPARQL SRC_LIST )
add_library( parser ${SRC_LIST} )