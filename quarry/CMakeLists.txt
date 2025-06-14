cmake_minimum_required(VERSION 3.4...3.18)
project(quarry LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

find_package(PostgreSQL REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Catch2 3 REQUIRED)
find_package(Boost CONFIG)
pkg_check_modules(PQXX REQUIRED libpqxx)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/migration)

# specifying each sources, explicitly
# idk why but globbing can be bad


file(GLOB_RECURSE SOURCES
    ${PROJECT_SOURCE_DIR}/include/**/*.h
    ${PROJECT_SOURCE_DIR}/src/**/*.cpp
)

add_executable(quarry_main ${SOURCES})

# 1. Locate header files
# 2. Locate binaries
# 3. Specify these binaries need to be linked to this executable


target_include_directories(quarry_main PRIVATE ${Boost_INCLUDE_DIRS})

# 1.
# links header files (fixes the include linter errors)
target_include_directories(quarry_main PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PQXX_INCLUDE_DIRS}
)

# 2.
# .dylib or .a (the compiled binaries)
target_link_directories(quarry_main PRIVATE
${PQXX_LIBRARY_DIRS}
)

# 3.
# links my executable to the actual libraries
target_link_libraries(quarry_main
${PostgreSQL_LIBRARIES}
${PQXX_LIBRARIES}
)



# testing information
# TODO move this to a root cmakelists when I start on the engine
# TODO Also look into cmakepresets.json

enable_testing()

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/tests/test_*.cpp)

foreach(TEST_SRC ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SRC} NAME_WE)

    add_executable(${TEST_NAME} ${TEST_SRC})

    target_include_directories(${TEST_NAME} PRIVATE
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

