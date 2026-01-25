cmake_minimum_required(VERSION 3.29)

set(QUARRY_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# ---- Dependencies ----

find_package(PostgreSQL REQUIRED)
find_package(glaze REQUIRED)
find_package(OpenSSL REQUIRED COMPONENTS SSL Crypto)
find_package(Threads REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(quill CONFIG REQUIRED)


set(_boost_components system)
find_package(Boost 1.82 CONFIG QUIET COMPONENTS ${_boost_components})
if(NOT Boost_FOUND)
  cmake_policy(SET CMP0167 OLD)
  find_package(Boost REQUIRED COMPONENTS ${_boost_components})
  set(QUARRY_BOOST_USE_IMPORT_TARGETS OFF)
else()
  set(QUARRY_BOOST_USE_IMPORT_TARGETS ON)
endif()

find_package(libpqxx CONFIG QUIET)
if(libpqxx_FOUND)
  set(LIBPQXX_TARGET libpqxx::pqxx)
else()
  find_package(unofficial-libpqxx CONFIG QUIET)
  if(unofficial-libpqxx_FOUND)
    set(LIBPQXX_TARGET unofficial::libpqxx::pqxx)
  else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(PQXX IMPORTED_TARGET REQUIRED libpqxx)
    set(LIBPQXX_TARGET PkgConfig::PQXX)
  endif()
endif()

add_library(quarry_logging INTERFACE)

target_include_directories(quarry_logging
  INTERFACE
    "${QUARRY_DIR}/include"
)

target_link_libraries(quarry_logging
  INTERFACE
    quill::quill
)

# ---- Targets & sources ----
file(GLOB_RECURSE QUARRY_API_SOURCES       CONFIGURE_DEPENDS "${QUARRY_DIR}/src/api/*.cpp")
file(GLOB_RECURSE QUARRY_DB_SOURCES CONFIGURE_DEPENDS "${QUARRY_DIR}/src/db/*.cpp" )

add_library(quarry_core STATIC
  ${QUARRY_API_SOURCES}
  ${QUARRY_DB_SOURCES}
)

target_include_directories(quarry_core
  PRIVATE
    "${QUARRY_DIR}/include"
    "${QUARRY_DIR}/include/db"
    "${QUARRY_DIR}/include/api"
    "${QUARRY_DIR}/include/api/endpoints"
)

add_library(quarry_internal INTERFACE)
target_link_libraries(quarry_internal INTERFACE quarry_core)
target_include_directories(quarry_internal
  INTERFACE
    "${QUARRY_DIR}/include"
    "${QUARRY_DIR}/include/db"
    "${QUARRY_DIR}/include/api"
    "${QUARRY_DIR}/include/api/endpoints"
)

add_library(quarry_api INTERFACE)
target_link_libraries(quarry_api INTERFACE quarry_core)
target_include_directories(quarry_api
  INTERFACE
    "${QUARRY_DIR}/include"
    "${QUARRY_DIR}/include/api"
    "${QUARRY_DIR}/include/api/endpoints"
)

# linking core to previously imported targets
target_link_libraries(quarry_core
  PUBLIC
    quarry_logging
    PostgreSQL::PostgreSQL
    glaze::glaze
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
)

if(QUARRY_BOOST_USE_IMPORT_TARGETS)
  target_link_libraries(quarry_core PUBLIC Boost::headers Boost::system)
else()
  # Legacy FindBoost variables
  target_include_directories(quarry_core PUBLIC ${Boost_INCLUDE_DIRS})
  target_link_libraries(quarry_core PUBLIC ${Boost_LIBRARIES})
endif()

target_link_libraries(quarry_core PUBLIC ${LIBPQXX_TARGET})

target_link_libraries(quarry_core INTERFACE)

# ---- Executables ----
add_executable(quarry_main "${QUARRY_DIR}/src/main.cpp")
target_link_libraries(quarry_main PRIVATE quarry_internal)

add_executable(quarry_bootstrap "${QUARRY_DIR}/src/bootstrap.cpp")
target_link_libraries(quarry_bootstrap PRIVATE quarry_internal)

add_executable(quarry_grpc_server "${QUARRY_DIR}/src/grpc_server.cpp")
target_link_libraries(quarry_grpc_server
  PRIVATE
    quarry_internal
    marble_protos
    gRPC::grpc++
    gRPC::grpc++_reflection
)


# -Wl,-ld_classic for https://stackoverflow.com/a/77190575/20313250
if(APPLE)
  target_link_options(quarry_core PRIVATE "-Wl,-no_warn_duplicate_libraries")
  target_link_options(quarry_main PRIVATE "-Wl,-no_warn_duplicate_libraries")
  target_link_options(quarry_bootstrap PRIVATE "-Wl,-no_warn_duplicate_libraries")
  target_link_options(quarry_grpc_server PRIVATE "-Wl,-no_warn_duplicate_libraries")
endif()

# ---- Tests ----
if(BUILD_TESTING)
  file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "${QUARRY_DIR}/tests/test_*.cpp")

  foreach(TEST_SRC IN LISTS TEST_SOURCES)
    get_filename_component(TEST_NAME "${TEST_SRC}" NAME_WE)

    add_executable(${TEST_NAME} "${TEST_SRC}")
    target_link_libraries(${TEST_NAME}
      PRIVATE
        quarry_internal
        Catch2::Catch2WithMain
    )

    catch_discover_tests(${TEST_NAME}
      TEST_PREFIX "[quarry] "
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
  endforeach()
endif()
