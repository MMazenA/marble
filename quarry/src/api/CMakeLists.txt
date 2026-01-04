cmake_minimum_required(VERSION 3.29)

file(GLOB QUARRY_API_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
target_sources(quarry_core PRIVATE ${QUARRY_API_SOURCES})
