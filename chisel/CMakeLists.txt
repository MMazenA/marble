cmake_minimum_required(VERSION 3.29)

set(CHISEL_DIR ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

add_library(chisel_core STATIC)

target_include_directories(chisel_core
  PUBLIC
    "${CHISEL_DIR}/include"
)

target_link_libraries(chisel_core
  PUBLIC
    marble_protos
)

add_subdirectory(src/engine)
add_subdirectory(src/strategy)

add_library(chisel_grpc STATIC)

target_include_directories(chisel_grpc
  PUBLIC
    "${CHISEL_DIR}/include"
)

target_link_libraries(chisel_grpc
  PUBLIC
    chisel_core
    marble_protos
    gRPC::grpc++
)

add_subdirectory(src/grpc)


add_executable(chisel_server "${CHISEL_DIR}/src/server_main.cpp")
target_link_libraries(chisel_server
  PRIVATE
    chisel_grpc
    quarry_logging
)

add_executable(chisel_client "${CHISEL_DIR}/src/grpc_client.cpp")
target_link_libraries(chisel_client
  PRIVATE
    quarry_logging
    marble_protos
    gRPC::grpc++
)

add_executable(main "${CHISEL_DIR}/src/main.cpp")
target_link_libraries(main
  PRIVATE
    quarry_api
)

# ============================================================================
# Tests
# ============================================================================
include(CTest)
include(Catch)

add_subdirectory(src/tests)
