add_library(gstore_database OBJECT
        Database.cpp
        Join.cpp
        CSR.cpp
        TableOperator.cpp
        PlanTree.cpp
        PlanGenerator.cpp
        Executor.cpp
        Optimizer.cpp
        Txn_manager.cpp
)

add_library(gcsr SHARED
        CSR.cpp
)
# set fPIC for gscr
set_property(TARGET gcsr PROPERTY POSITION_INDEPENDENT_CODE ON)
add_dependencies(gcsr prepare)
install(TARGETS gcsr DESTINATION ${GSTORE_LIB_DIR})