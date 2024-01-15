add_library(gstore_topk OBJECT
        topk/DPB/DynamicTrie.cpp
        topk/DPB/OrderedList.cpp
        topk/DPB/Pool.cpp
        topk/DPBTopKUtil.cpp
        topk/TopKSearchPlan.cpp
        topk/TopKUtil.cpp
)

add_library(gstore_query OBJECT
        SPARQLquery.cpp
        BasicQuery.cpp
        ResultSet.cpp
        IDList.cpp
        DFSPlan.cpp
        Varset.cpp
        QueryTree.cpp
        TempResult.cpp
        QueryCache.cpp
        GeneralEvaluation.cpp
        PathQueryHandler.cpp
        BGPQuery.cpp
        FilterPlan.cpp
)

add_library(gpathqueryhandler SHARED
        PathQueryHandler.cpp
)
# set fPIC
set_property(TARGET gpathqueryhandler PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(gpathqueryhandler gcsr)
add_dependencies(gpathqueryhandler gcsr prepare)
install(TARGETS gpathqueryhandler DESTINATION ${GSTORE_LIB_DIR})