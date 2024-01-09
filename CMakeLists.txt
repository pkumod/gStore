# SPDX-License-Identifier: BSD-3-Clause
# @authors: Runzhi He <rzhe@pku.edu.cn>
# @date: 2023-12-20

cmake_minimum_required(VERSION 3.16)

project(gStore LANGUAGES CXX)

# gStore requires gcc 8 or higher
if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_VERSION VERSION_LESS "8")
    message(FATAL_ERROR "gStore requires gcc 8 or higher, got ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
endif()

### set compiler flags
# use -O2 for Release
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
# ignore unused-result, panic on return-type
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-result -Werror=return-type")
# check CMAKE_BUILD_TYPE, if not set, set to Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
    message(WARNING "Build type not set, default to Release")
endif()
# Check if mold is available and set it as the linker
find_program(MOLD_LINKER mold)
if(MOLD_LINKER)
    message(STATUS "mold linker found: ${MOLD_LINKER}, adding -fuse-ld=mold to linker flags")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=mold")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=mold")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -fuse-ld=mold")
else()
    message(STATUS "mold linker not found, using default linker")
endif()
# coverage settings
if(COVERAGE)
    message(STATUS "Enabling coverage reporting")
    # halt if the build_type is not Debug
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(FATAL_ERROR "Coverage requires Debug build type")
    endif()
    # set compiler flags
    add_link_options(--coverage)
    add_compile_options(--coverage)
endif ()

# print build type and flags
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "CXX Flags: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}}")

# helper for IDEs to find the include path
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add include/ and lib/
include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_SOURCE_DIR}/lib)
# TODO: The line below is a workaround for millions of relative includes. Remove it when all relative includes are replaced with target_include_directories.
include_directories(${CMAKE_INCLUDE_PATH})

# Set output directories (use -DGSTORE_ROOT_DIR=... to override)
SET(GSTORE_ROOT_DIR ${CMAKE_SOURCE_DIR})
SET(GSTORE_EXE_DIR ${GSTORE_ROOT_DIR}/bin)
SET(GSTORE_LIB_DIR ${GSTORE_ROOT_DIR}/lib)
SET(GSTORE_TEST_DIR ${GSTORE_ROOT_DIR}/bin_tests)

# if option USE_DYNAMIC_LIB is set, set env var GSTORE_USE_DYNAMIC_LIB to 1
SET(USE_DYNAMIC_LIB ON CACHE BOOL "Use dynamic libraries")
if(USE_DYNAMIC_LIB OR ENV{GSTORE_USE_DYNAMIC_LIB})
    set(ENV{GSTORE_USE_DYNAMIC_LIB} 1)
    # Set rpath so that gStore can find its shared libraries
    set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
endif()

### Dependencies managed by conan
# boost-system, boost-regex, boost-thread
find_package(Boost REQUIRED CONFIG COMPONENTS system regex thread)
# minizip: minizip::minizip, minizip/zip.h
find_package(minizip REQUIRED CONFIG)
# openssl: OpenSSL::SSL, OpenSSL::Crypto
find_package(OpenSSL REQUIRED CONFIG)
# indicators: indicators::indicators, indicators/progress_bar.hpp
find_package(indicators REQUIRED CONFIG)
## antlr4-runtime: antlr4_shared, antlr4_static
#find_package(antlr4-runtime REQUIRED CONFIG)
# rapidjson: rapidjson, rapidjson/document.h
find_package(RapidJSON REQUIRED CONFIG)
# logcplus: log4cplus::log4cplus, log4cplus/logger.h
find_package(log4cplus REQUIRED CONFIG)
# curl: CURL::libcurl, curl/curl.h
find_package(CURL REQUIRED CONFIG)

### Dependencies managed by system package manager
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
# readline: have to link for license compatibility with GPL-3
find_library(LIB_READLINE NAMES readline REQUIRED)
# jemalloc
find_library(LIB_JEMALLOC NAMES jemalloc REQUIRED)
# openmp: OpenMP::OpenMP_CXX  # the elegance below requires cmake 3.9 or higher
find_package(OpenMP REQUIRED)
# uuid: uuid::uuid (required by antlr4-cppruntime)
find_library(LIB_UUID NAMES uuid REQUIRED)
# Backward: Backward::Backward
find_package(Backward REQUIRED)
# libdwarf: libdwarf::libdwarf
find_package(libdwarf REQUIRED)
# libelf libelf::libelf
find_package(libelf REQUIRED)

### Prepare unmanaged dependencies
add_custom_target(
        prepare
        COMMAND [ -d ${CMAKE_SOURCE_DIR}/include ] && [ -d ${CMAKE_SOURCE_DIR}/lib ] || bash ${CMAKE_SOURCE_DIR}/3rdparty/init.sh
        COMMENT "Prepare unmanaged dependencies"
)

add_custom_target(
        clean_prepare
        COMMAND rm -rvf ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/lib/*.a
        COMMAND find ${CMAKE_SOURCE_DIR}/3rdparty -type d -mindepth 1 -maxdepth 1 -exec rm -rvf {} +
        COMMENT "Clean unmanaged dependencies"
)

add_custom_target(
        uninstall
        # remove all files in bin/, lib/, and scripts/ that are not *.sh or *.py
        COMMAND rm -rvf ${GSTORE_EXE_DIR}
        COMMAND rm -rvf ${GSTORE_LIB_DIR}/libgcsr.so ${GSTORE_LIB_DIR}/libgpathqueryhandler.so
        COMMAND find ${GSTORE_TEST_DIR} -type f -not -name '*.*' -exec rm -v {} +
        COMMENT "Clean installed files"
)

add_custom_target(
        tarball
        COMMAND cd ../ && ${CMAKE_COMMAND} -E tar "cfzv" "gstore-src.tar.gz" --
        "src" "api" "defaults" "tests" "3rdparty" "data" "scripts" "conanfile.py" "CMakeLists.txt" "README.md" "LICENSE"
        COMMENT "Creating source tarball"
        VERBATIM
)

string(TOLOWER "${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}" GSTORE_PLATFORM_NAME)
add_custom_target(
        package
        COMMAND cd ../ && ${CMAKE_COMMAND} -E tar "cfzv" "gstore-${GSTORE_PLATFORM_NAME}.tar.gz" --
        "bin" "lib" "defaults" "data/system" "scripts" "README.md" "LICENSE"
        COMMENT "Creating binary tarball"
        VERBATIM
)

# ignored files for lcov (*Server* to wildcard all server files)
SET(GSTORE_COVERAGE_IGNORES "*Server*" "*Main*" "*Parser/SPARQL*" "*include/*" "*tests/*")
if (COVERAGE)
    add_custom_target(
            coverage
            COMMAND lcov --capture --directory ${CMAKE_SOURCE_DIR} --output-file ${CMAKE_SOURCE_DIR}/coverage.info
            # remove dependencies
            COMMAND lcov --remove ${CMAKE_SOURCE_DIR}/coverage.info "/usr/*" "*conan*" --output-file ${CMAKE_SOURCE_DIR}/coverage.info
            # remove untested
            COMMAND lcov --remove ${CMAKE_SOURCE_DIR}/coverage.info ${GSTORE_COVERAGE_IGNORES} --output-file ${CMAKE_SOURCE_DIR}/coverage.info
            COMMAND genhtml ${CMAKE_SOURCE_DIR}/coverage.info --output-directory ${CMAKE_SOURCE_DIR}/temp --demangle-cpp --frames --legend  --show-details --title "Code Coverage"
            COMMAND lcov --list ${CMAKE_SOURCE_DIR}/coverage.info
            COMMAND ${CMAKE_COMMAND} -E tar -czf  ${CMAKE_SOURCE_DIR}/coverage.tar.gz -C ${CMAKE_SOURCE_DIR}/temp .
            COMMENT "Generating code coverage report -> temp/index.html, coverage.tar.gz"
            VERBATIM
    )
endif ()

# Add large file support on 32-bit builds
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    add_definitions(-D_FILE_OFFSET_BITS=64)
    add_definitions(-D_LARGEFILE_SOURCE)
    add_definitions(-D_LARGEFILE64_SOURCE)
endif()

add_subdirectory(src)
add_subdirectory(tests)

# rules for ctest
include(CTest)
enable_testing()
add_test(NAME BasicTest COMMAND bash ${CMAKE_SOURCE_DIR}/tests/basic_test.sh)
add_test(NAME ParserTest COMMAND bash ${CMAKE_SOURCE_DIR}/tests/parser_test.sh)
add_test(NAME DatasetTest COMMAND dataset_test)
add_test(NAME UpdateTest COMMAND update_test 100)
add_test(NAME TransactionTest COMMAND transaction_test)
add_test(NAME RunTransaction COMMAND run_transaction)
add_test(NAME DebugTest COMMAND debug_test)

set(GSTORE_CI_TESTS
        BasicTest
        ParserTest
        DatasetTest
        UpdateTest
        TransactionTest
        RunTransaction
        DebugTest
)

foreach (test ${GSTORE_CI_TESTS})
    set_tests_properties(${test} PROPERTIES TIMEOUT 300 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endforeach ()
