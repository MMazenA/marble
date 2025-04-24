cmake_minimum_required(VERSION 3.4...3.18)
project(Vector LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)


include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(quarry_main src/bootstrap.cpp ${SOURCES})

