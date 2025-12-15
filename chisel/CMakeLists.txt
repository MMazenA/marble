cmake_minimum_required(VERSION 3.29)

set(CHISEL_DIR ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

add_executable(chisel_client "${CHISEL_DIR}/src/main.cpp")

target_link_libraries(chisel_client
	PRIVATE
		marble_protos
		gRPC::grpc++
)

target_include_directories(chisel_client
	PRIVATE
		"${CHISEL_DIR}/include"
)
