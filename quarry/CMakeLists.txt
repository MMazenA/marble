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
find_package(Boost 1.82 CONFIG REQUIRED COMPONENTS system)
find_package(unofficial-libpqxx CONFIG REQUIRED)

add_library(quarry_logging INTERFACE)

target_include_directories(quarry_logging
  INTERFACE
    "${QUARRY_DIR}/include"
)

target_link_libraries(quarry_logging
  INTERFACE
    quill::quill
)

add_library(quarry_core STATIC)

target_include_directories(quarry_core
  PUBLIC
    "${QUARRY_DIR}/include"
    "${QUARRY_DIR}/include/db"
    "${QUARRY_DIR}/include/api"
    "${QUARRY_DIR}/include/api/endpoints"
)

add_subdirectory(src)

# linking core to previously imported targets
target_link_libraries(quarry_core
  PUBLIC
    quarry_logging
    PostgreSQL::PostgreSQL
    glaze::glaze
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
    Boost::headers
    Boost::system
    unofficial::libpqxx::pqxx
)

target_link_libraries(quarry_core INTERFACE)

# ---- Executables ----
add_executable(quarry_main "${QUARRY_DIR}/src/main.cpp")
target_link_libraries(quarry_main PRIVATE quarry_core)

add_executable(quarry_bootstrap "${QUARRY_DIR}/src/bootstrap.cpp")
target_link_libraries(quarry_bootstrap PRIVATE quarry_core)

add_executable(quarry_grpc_server "${QUARRY_DIR}/src/grpc_server.cpp")
target_link_libraries(quarry_grpc_server
  PRIVATE
    quarry_core
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
  add_subdirectory(tests)
endif()
