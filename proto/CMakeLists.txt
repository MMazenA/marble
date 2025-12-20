set(PROTO_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(PROTO_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)

find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

add_library(marble_protos STATIC)

target_include_directories(marble_protos
  PUBLIC
    ${PROTO_ROOT}
    ${PROTO_GENERATED_DIR}
)

file(GLOB PROTO_FILES CONFIGURE_DEPENDS "${PROTO_ROOT}/*.proto")

protobuf_generate(
  TARGET marble_protos
  LANGUAGE cpp
  PROTOS ${PROTO_FILES}
  IMPORT_DIRS ${PROTO_ROOT}
  PROTOC_OUT_DIR ${PROTO_GENERATED_DIR}
)

protobuf_generate(
  TARGET marble_protos
  LANGUAGE grpc
  GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
  PLUGIN "protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>"
  PROTOS ${PROTO_FILES}
  IMPORT_DIRS ${PROTO_ROOT}
  PROTOC_OUT_DIR ${PROTO_GENERATED_DIR}
)

target_link_libraries(marble_protos
  PUBLIC
    protobuf::libprotobuf
    gRPC::grpc++
)
