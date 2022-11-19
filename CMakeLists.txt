cmake_minimum_required(VERSION 3.1)
project("interface")

# generate .so file

cmake_minimum_required(VERSION 3.1)
execute_process(
        COMMAND bash -c "find ${PROJECT_SOURCE_DIR}/../.. -type d -regex \".*output/include$\" | head -n1 | xargs dirname | tr -d '\n'"
        OUTPUT_VARIABLE OUTPUT_PATH
)

set(CMAKE_PREFIX_PATH ${OUTPUT_PATH})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++11 ")

message(${CMAKE_CXX_FLAGS})



# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.



option(LINK_SO "Whether examples are linked dynamically" OFF)

set(CMAKE_PREFIX_PATH ${OUTPUT_PATH})
include(FindThreads)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
set(INC_DIR
        ${PROJECT_SOURCE_DIR}/inc
        ${PROJECT_SOURCE_DIR}/incubator-brpc/gflags/include
        ${PROJECT_SOURCE_DIR}/incubator-brpc/protobuf/include
        ${PROJECT_SOURCE_DIR}/incubator-brpc/leveldb/include
        ${PROJECT_SOURCE_DIR}/incubator-brpc/snappy/include
        ${PROJECT_SOURCE_DIR}/incubator-brpc/openssl/include
        ${PROJECT_SOURCE_DIR}/incubator-brpc/zlib/include)
include_directories(${INC_DIR})
find_library(THRIFT_LIB NAMES thrift)
if (NOT THRIFT_LIB)
    set(THRIFT_LIB "")
endif()
find_library(THRIFTNB_LIB NAMES thriftnb)
if (NOT THRIFTNB_LIB)
    set(THRIFTNB_LIB "")
endif()

find_path(BRPC_INCLUDE_PATH NAMES brpc/server.h)
if(LINK_SO)
    find_library(BRPC_LIB NAMES brpc)
else()
    find_library(BRPC_LIB NAMES libbrpc.a brpc)
endif()
if((NOT BRPC_INCLUDE_PATH) OR (NOT BRPC_LIB))
    message(FATAL_ERROR "Fail to find brpc")
endif()
include_directories(${BRPC_INCLUDE_PATH})

set(GFLAGS_INCLUDE_PATH ./incubator-brpc/gflags/include)

set(CMAKE_CPP_FLAGS "${DEFINE_CLOCK_GETTIME} -DGFLAGS_NS=${GFLAGS_NS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CPP_FLAGS} -DNDEBUG -O2 -D__const__=__unused__ -pipe -W -Wall -Wno-unused-parameter -fPIC -fno-omit-frame-pointer")

set(DYNAMIC_LIB
        ${CMAKE_THREAD_LIBS_INIT}
        dl
        )
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(DYNAMIC_LIB ${DYNAMIC_LIB}
            pthread
            "-framework CoreFoundation"
            "-framework CoreGraphics"
            "-framework CoreData"
            "-framework CoreText"
            "-framework Security"
            "-framework Foundation"
            "-Wl,-U,_MallocExtension_ReleaseFreeMemory"
            "-Wl,-U,_ProfilerStart"
            "-Wl,-U,_ProfilerStop")
endif()



add_executable(client
        src/test_client.cpp
        )
add_executable(server
        src/test_server.cpp
        )
add_library(interface
        SHARED
        src/data.pb.cc
        src/interface.cpp
        src/qps.cpp)

target_link_libraries(${PROJECT_NAME} pmemobj)
target_link_libraries(${PROJECT_NAME} pmem)
target_link_libraries(${PROJECT_NAME} ${BRPC_LIB} ${DYNAMIC_LIB})

target_link_libraries(${PROJECT_NAME} "${PROJECT_SOURCE_DIR}/incubator-brpc/protobuf/lib/libprotobuf.a")
target_link_libraries(${PROJECT_NAME} "${PROJECT_SOURCE_DIR}/incubator-brpc/zlib/lib/libz.a")
target_link_libraries(${PROJECT_NAME} "${PROJECT_SOURCE_DIR}/incubator-brpc/gflags/lib/libgflags.a")
target_link_libraries(${PROJECT_NAME} "${PROJECT_SOURCE_DIR}/incubator-brpc/openssl/lib/libssl.a")
target_link_libraries(${PROJECT_NAME} "${PROJECT_SOURCE_DIR}/incubator-brpc/openssl/lib/libcrypto.a")
target_link_libraries(${PROJECT_NAME} "${PROJECT_SOURCE_DIR}/incubator-brpc/leveldb/lib/libleveldb.a")
target_link_libraries(${PROJECT_NAME} "${PROJECT_SOURCE_DIR}/incubator-brpc/snappy/lib/libsnappy.a")

target_link_libraries(client
        PRIVATE
        interface)
target_link_libraries(server
        PRIVATE
        interface)


