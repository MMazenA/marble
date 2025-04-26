cmake_minimum_required(VERSION 3.4...3.18)
project(quarry LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

find_package(PostgreSQL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(quarry_main src/bootstrap.cpp ${SOURCES})

target_include_directories(quarry_main PRIVATE
    ${PostgreSQL_INCLUDE_DIRS}
    ${PQXX_INCLUDE_DIRS}
)

target_link_directories(quarry_main PRIVATE
    ${PQXX_LIBRARY_DIRS}
)

target_link_libraries(quarry_main
    ${PostgreSQL_LIBRARIES}
    ${PQXX_LIBRARIES}
)
