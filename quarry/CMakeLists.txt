cmake_minimum_required(VERSION 3.29)

set(QUARRY_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# ---- Dependencies ----

find_package(PostgreSQL REQUIRED)
find_package(OpenSSL REQUIRED COMPONENTS SSL Crypto)
find_package(Threads REQUIRED)


set(_boost_components system)
find_package(Boost 1.82 CONFIG QUIET COMPONENTS ${_boost_components})
if(NOT Boost_FOUND)
  cmake_policy(SET CMP0167 OLD)
  find_package(Boost REQUIRED COMPONENTS ${_boost_components})
  set(QUARRY_BOOST_USE_IMPORT_TARGETS OFF)
else()
  set(QUARRY_BOOST_USE_IMPORT_TARGETS ON)
endif()

find_package(unofficial-libpqxx CONFIG QUIET)
set(HAVE_UNOFFICIAL_LIBPQXX ${unofficial-libpqxx_FOUND})

if(NOT HAVE_UNOFFICIAL_LIBPQXX)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(PQXX IMPORTED_TARGET REQUIRED libpqxx)
endif()

# ---- Targets & sources ----
file(GLOB_RECURSE QUARRY_API_SOURCES       CONFIGURE_DEPENDS "${QUARRY_DIR}/src/api/*.cpp")
file(GLOB_RECURSE QUARRY_MIGRATION_SOURCES CONFIGURE_DEPENDS "${QUARRY_DIR}/src/migration/*.cpp" )

add_library(quarry_core STATIC
  ${QUARRY_API_SOURCES}
  ${QUARRY_MIGRATION_SOURCES}
)

target_include_directories(quarry_core
  PUBLIC
    "${QUARRY_DIR}/include"
    "${QUARRY_DIR}/include/migration"
    "${QUARRY_DIR}/include/api"
    "${QUARRY_DIR}/include/api/endpoints"
)

# Link dependencies via imported targets
target_link_libraries(quarry_core
  PUBLIC
    PostgreSQL::PostgreSQL
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
)


# Boost linkage (prefer imported targets if we found CONFIG)
if(QUARRY_BOOST_USE_IMPORT_TARGETS)
  target_link_libraries(quarry_core PUBLIC Boost::headers Boost::system)
else()
  # Legacy FindBoost variables
  target_include_directories(quarry_core PUBLIC ${Boost_INCLUDE_DIRS})
  target_link_libraries(quarry_core PUBLIC ${Boost_LIBRARIES})
endif()

# libpqxx (config or pkg-config target)
if(HAVE_UNOFFICIAL_LIBPQXX)
  target_link_libraries(quarry_core PUBLIC unofficial::libpqxx::pqxx)
else()
  target_link_libraries(quarry_core PUBLIC PkgConfig::PQXX)
endif()

#anything that links to to quarry_core will have to inherit my standards
add_library(marble_warnings INTERFACE)
target_compile_features(marble_warnings INTERFACE cxx_std_23)

target_compile_options(marble_warnings INTERFACE
$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic -Wconversion>
$<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
)

target_link_libraries(quarry_core INTERFACE marble_warnings)

# ---- Executables ----

add_executable(quarry_main "${QUARRY_DIR}/src/main.cpp")
target_link_libraries(quarry_main PRIVATE quarry_core marble_warnings)

add_executable(quarry_bootstrap "${QUARRY_DIR}/src/bootstrap.cpp")
target_link_libraries(quarry_bootstrap PRIVATE quarry_core marble_warnings)


# -Wl,-ld_classic for https://stackoverflow.com/a/77190575/20313250
if(APPLE)
  target_link_options(quarry_core PRIVATE "-Wl,-ld_classic")
  target_link_options(quarry_main PRIVATE "-Wl,-ld_classic")
  target_link_options(quarry_bootstrap PRIVATE "-Wl,-ld_classic")
endif()

# ---- Tests ----
if(BUILD_TESTING)
  file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "${QUARRY_DIR}/tests/test_*.cpp")

  foreach(TEST_SRC IN LISTS TEST_SOURCES)
    get_filename_component(TEST_NAME "${TEST_SRC}" NAME_WE)

    add_executable(${TEST_NAME} "${TEST_SRC}")
    target_link_libraries(${TEST_NAME}
      PRIVATE
        quarry_core
        Catch2::Catch2WithMain
        marble_warnings
    )

    # Auto-register tests with CTest
    catch_discover_tests(${TEST_NAME}
      TEST_PREFIX "[quarry] "
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
  endforeach()
endif()
