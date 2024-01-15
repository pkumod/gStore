add_library(gstore_util OBJECT
        Util.cpp
        WebUrl.cpp
        INIParser.cpp
        Slog.cpp
        Stream.cpp
        Bstr.cpp
        Triple.cpp
        VList.cpp
        EvalMultitypeValue.cpp
        Version.cpp
        SpinLock.cpp
        GraphLock.cpp
        Transaction.cpp
        IDTriple.cpp
        Latch.cpp
        IPWhiteList.cpp
        IPBlackList.cpp
        OrderedVector.cpp
        CompressFileUtil.cpp
        Backward.cpp
)

# set definition if has debug info
if (CMAKE_BUILD_TYPE MATCHES "Deb")
    target_compile_definitions(gstore_util PRIVATE BACKWARD_HAS_DWARF=1)
endif ()