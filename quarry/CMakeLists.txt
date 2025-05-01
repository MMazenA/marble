cmake_minimum_required(VERSION 3.4...3.18)
project(quarry LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

find_package(PostgreSQL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

include_directories(${PROJECT_SOURCE_DIR}/include)

# specifying each sources, explicitly
# idk why but globbing can be bad
set(SOURCES
    ${PROJECT_SOURCE_DIR}/include/migration.h
    ${PROJECT_SOURCE_DIR}/src/bootstrap.cpp
)

add_executable(quarry_main src/bootstrap.cpp ${SOURCES})

# 1. Locate header files
# 2. Locate binaries
# 3. Specify these binaries need to be linked to this executable


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

