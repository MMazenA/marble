cmake_minimum_required(VERSION 3.29)
project(marble VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CTest)

if(BUILD_TESTING)
  find_package(Catch2 3 CONFIG REQUIRED)
  include(Catch)
endif()


add_subdirectory(proto)
add_subdirectory(quarry)
add_subdirectory(chisel)
