cmake_minimum_required(VERSION 3.29)

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*_test.cpp")

foreach(TEST_SRC IN LISTS TEST_SOURCES)
  get_filename_component(TEST_NAME "${TEST_SRC}" NAME_WE)

  add_executable(${TEST_NAME} "${TEST_SRC}")
  target_link_libraries(${TEST_NAME}
    PRIVATE
      chisel_core
      Catch2::Catch2WithMain
  )

  catch_discover_tests(${TEST_NAME}
    TEST_PREFIX "[chisel] "
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )
endforeach()

