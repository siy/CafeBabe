cmake_minimum_required(VERSION 3.1)
project(test)

include_directories(../cpp-peglib)

set(CMAKE_CXX_STANDARD 17)

add_custom_command(
        OUTPUT grammar.cpp
        COMMAND ../../assemble.sh > ${CMAKE_CURRENT_BINARY_DIR}/grammar.cpp
        DEPENDS ../CafeBabe.peg
)

add_executable(test-main test-main.cc test1.cc test2.cc test3.cc ${CMAKE_CURRENT_BINARY_DIR}/grammar.cpp)
target_link_libraries(test-main ${add_link_deps})

add_test(
  NAME TestMain
  COMMAND test-main
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
