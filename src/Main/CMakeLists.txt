set(LIB_EXE
        # Conan Deps
        Boost::system
        Boost::regex
        Boost::thread
        minizip::minizip
        OpenSSL::SSL
        OpenSSL::Crypto
        indicators::indicators
        rapidjson
        log4cplus::log4cplus
        CURL::libcurl
        # System Deps
        OpenMP::OpenMP_CXX
        ${LIB_JEMALLOC}
        ${LIB_READLINE}
        Threads::Threads
        # Unmanaged Deps
        antlr4-runtime
        workflow
)

set(OBJ_EXE
        $<TARGET_OBJECTS:gstore_stringindex>
        $<TARGET_OBJECTS:gstore_parser>
        $<TARGET_OBJECTS:gstore_server>
        $<TARGET_OBJECTS:gstore_database>
        $<TARGET_OBJECTS:gstore_util>
        $<TARGET_OBJECTS:gstore_topk>
        $<TARGET_OBJECTS:gstore_query>
        $<TARGET_OBJECTS:gstore_trie>
        $<TARGET_OBJECTS:gstore_kvstore>
)

add_executable(gadd gadd.cpp ${OBJ_EXE})
target_link_libraries(gadd ${LIB_EXE})

add_executable(gsub gsub.cpp ${OBJ_EXE})
target_link_libraries(gsub ${LIB_EXE})

add_executable(gexport gexport.cpp ${OBJ_EXE})
target_link_libraries(gexport ${LIB_EXE})

add_executable(gdrop gdrop.cpp ${OBJ_EXE})
target_link_libraries(gdrop ${LIB_EXE})

add_executable(ginit ginit.cpp ${OBJ_EXE})
target_link_libraries(ginit ${LIB_EXE})

add_executable(shutdown shutdown.cpp $<TARGET_OBJECTS:gstore_connector> $<TARGET_OBJECTS:gstore_util>)
target_link_libraries(shutdown ${LIB_EXE})

add_executable(gmonitor gmonitor.cpp ${OBJ_EXE})
target_link_libraries(gmonitor ${LIB_EXE})

add_executable(gshow gshow.cpp ${OBJ_EXE})
target_link_libraries(gshow ${LIB_EXE})

add_executable(gbuild gbuild.cpp ${OBJ_EXE})
target_link_libraries(gbuild ${LIB_EXE})

add_executable(gquery gquery.cpp ${OBJ_EXE})
# seems gquery does need readline
target_compile_definitions(gquery PRIVATE READLINE_ON)
target_link_libraries(gquery ${LIB_EXE})

add_executable(gserver gserver.cpp ${OBJ_EXE})
target_link_libraries(gserver ${LIB_EXE})

add_executable(gserver_backup_scheduler gserver_backup_scheduler.cpp ${OBJ_EXE})
target_link_libraries(gserver_backup_scheduler ${LIB_EXE})

add_executable(ghttp ghttp.cpp ${OBJ_EXE} $<TARGET_OBJECTS:gstore_server> $<TARGET_OBJECTS:gstore_grpc>)
target_link_libraries(ghttp ${LIB_EXE})

add_executable(grpc grpc.cpp ${OBJ_EXE} $<TARGET_OBJECTS:gstore_grpc>)
target_link_libraries(grpc ${LIB_EXE} workflow OpenSSL::SSL OpenSSL::Crypto)

add_executable(gbackup gbackup.cpp ${OBJ_EXE})
target_link_libraries(gbackup ${LIB_EXE})

add_executable(grestore grestore.cpp ${OBJ_EXE})
target_link_libraries(grestore ${LIB_EXE})

add_executable(gpara gpara.cpp ${OBJ_EXE} $<TARGET_OBJECTS:gstore_connector>)
target_link_libraries(gpara ${LIB_EXE})

add_executable(rollback rollback.cpp ${OBJ_EXE} $<TARGET_OBJECTS:gstore_connector>)
target_link_libraries(rollback ${LIB_EXE})

add_executable(gconsole gconsole.cpp ${OBJ_EXE})
target_link_libraries(gconsole ${LIB_EXE})

SET(gstore_binaries
        gadd
        gsub
        gexport
        gdrop
        ginit
        shutdown
        gmonitor
        gshow
        gbuild
        gquery
        gserver
        gserver_backup_scheduler
        ghttp
        grpc
        gbackup
        grestore
        gpara
        rollback
        gconsole
)

foreach (gstore_binary ${gstore_binaries})
    install(TARGETS ${gstore_binary} DESTINATION ${GSTORE_EXE_DIR})
endforeach ()