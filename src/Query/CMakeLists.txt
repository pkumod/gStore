include_directories( ${INCLUDE_3RD_PATH} )
aux_source_directory( . SRC_LIST )
aux_source_directory( ./topk SRC_LIST )
aux_source_directory( ./topk/DPB SRC_LIST )
aux_source_directory( ../Database SRC_LIST )
add_library( query ${SRC_LIST} )

target_link_libraries( query
   database
   dl
)