# the reason cmake list ff
include_directories( ${INCLUDE_3RD_PATH} )
aux_source_directory( . SRC_LIST )
add_library( reason ${SRC_LIST} )
target_link_libraries( reason
   database
   dl
)
