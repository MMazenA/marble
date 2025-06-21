cmake_minimum_required(VERSION 3.4...3.18)
project(quarry LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

# Dependencies
find_package(PostgreSQL REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Catch2 3 REQUIRED)
find_package(Boost CONFIG)
pkg_check_modules(PQXX REQUIRED libpqxx)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/migration)

# === Source grouping ===

# Common sources (shared between executables)
file(GLOB_RECURSE COMMON_SOURCES
    ${PROJECT_SOURCE_DIR}/src/api/*.cpp
    ${PROJECT_SOURCE_DIR}/src/migration/*.cpp
)

# quarry_main (main app)
set(QUARRY_MAIN_SOURCES
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    ${COMMON_SOURCES}
)

# quarry_bootstrap (initial migration setup)
set(QUARRY_BOOTSTRAP_SOURCES
    ${PROJECT_SOURCE_DIR}/src/bootstrap.cpp
    ${COMMON_SOURCES}
)

# === Executable: quarry_main ===
add_executable(quarry_main ${QUARRY_MAIN_SOURCES})

target_include_directories(quarry_main PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PQXX_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

target_link_directories(quarry_main PRIVATE
    ${PQXX_LIBRARY_DIRS}
)

target_link_libraries(quarry_main
    ${PostgreSQL_LIBRARIES}
    ${PQXX_LIBRARIES}
)

# === Executable: quarry_bootstrap ===
add_executable(quarry_bootstrap ${QUARRY_BOOTSTRAP_SOURCES})

target_include_directories(quarry_bootstrap PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PQXX_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

target_link_directories(quarry_bootstrap PRIVATE
    ${PQXX_LIBRARY_DIRS}
)

target_link_libraries(quarry_bootstrap
    ${PostgreSQL_LIBRARIES}
    ${PQXX_LIBRARIES}
)

# === Testing ===
enable_testing()

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/tests/test_*.cpp)

foreach(TEST_SRC ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SRC} NAME_WE)

    add_executable(${TEST_NAME} ${TEST_SRC} ${COMMON_SOURCES})

    target_include_directories(${TEST_NAME} PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/include/api
        ${PROJECT_SOURCE_DIR}/include/migration
        ${PQXX_INCLUDE_DIRS}
    )

    target_link_directories(${TEST_NAME} PRIVATE ${PQXX_LIBRARY_DIRS})

    target_link_libraries(${TEST_NAME}
        Catch2::Catch2WithMain
        ${PostgreSQL_LIBRARIES}
        ${PQXX_LIBRARIES}
    )

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()
