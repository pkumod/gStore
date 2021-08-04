cmake_minimum_required(VERSION 3.6)

set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "build type")

project(tutorial
		LANGUAGES C CXX
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

set(library "-L../lib -lreadline -L/usr/local/lib -L/usr/lib/ -L/usr/local/lib -L/usr/lib/ -lantlr4-runtime -lgcov   -lpthread  -lcurl")


find_library(LIBRT rt)
find_package(OpenSSL REQUIRED)
find_package(workflow REQUIRED CONFIG HINTS ..)
include_directories(${OPENSSL_INCLUDE_DIR} ${WORKFLOW_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../tools/antlr4-cpp-runtime-4/runtime/src)
link_directories(${WORKFLOW_LIB_DIR} "../lib")


if (WIN32)
		set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   /MP /wd4200")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /wd4200 /std:c++14")
else ()
		set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Wall -fPIC -pipe -std=gnu90")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC -pipe -std=c++11 -fexceptions ${library}")
endif ()

set(TUTORIAL_LIST
    tutorial-01-wget
	tutorial-04-apiserver
	tutorial-05-http_proxy
	tutorial-06-parallel_wget
	tutorial-07-sort_task
	tutorial-08-matrix_multiply
	tutorial-09-http_file_server
	tutorial-11-graph_task
)

if (APPLE)
	set(WORKFLOW_LIB workflow pthread OpenSSL::SSL OpenSSL::Crypto)
else ()
	set(WORKFLOW_LIB workflow ${LIBRT})
endif ()

foreach(src ${TUTORIAL_LIST})
	string(REPLACE "-" ";" arr ${src})
	list(GET arr -1 bin_name)
	add_executable(${bin_name} ${src}.cc)
	target_link_libraries(${bin_name} ${WORKFLOW_LIB})
endforeach()

if (NOT REDIS STREQUAL "n")
set(TUTORIAL_LIST
	tutorial-02-redis_cli
	tutorial-03-wget_to_redis
)
foreach(src ${TUTORIAL_LIST})
	string(REPLACE "-" ";" arr ${src})
	list(GET arr -1 bin_name)
	add_executable(${bin_name} ${src}.cc)
	target_link_libraries(${bin_name} ${WORKFLOW_LIB})
endforeach()
endif()

if (NOT MYSQL STREQUAL "n")
set(TUTORIAL_LIST
	tutorial-12-mysql_cli
)
foreach(src ${TUTORIAL_LIST})
	string(REPLACE "-" ";" arr ${src})
	list(GET arr -1 bin_name)
	add_executable(${bin_name} ${src}.cc)
	target_link_libraries(${bin_name} ${WORKFLOW_LIB})
endforeach()
endif()

if (KAFKA STREQUAL "y")
	add_executable("kafka_cli" "tutorial-13-kafka_cli.cc")
	target_link_libraries("kafka_cli" wfkafka workflow z snappy lz4 zstd rt)
endif ()




