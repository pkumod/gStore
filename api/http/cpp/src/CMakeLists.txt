cmake_minimum_required(VERSION 3.4)

project(gStoreConnector)
# try to find curl by conan
if (NOT CURL_FOUND)
    find_package(CURL QUIET)
endif ()
# if can't find curl by conan, try to find libcurl by pkg-config
if (NOT CURL_FOUND)
    find_package(PkgConfig REQUIRED)
    pkg_search_module(CURL REQUIRED)
endif()

set(CONNECTOR_SOURCES GstoreConnector.cpp)

add_library(gstore_connector OBJECT GstoreConnector.cpp)

add_library(gstore_connector_static STATIC $<TARGET_OBJECTS:gstore_connector>)
target_link_libraries(gstore_connector_static ${CURL_LIBRARIES})