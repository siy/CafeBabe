cmake_minimum_required(VERSION 3.16)
project(CafeBabe)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything -Wno-c++98-compat -Wno-padded -Wno-weak-vtables -Wno-exit-time-destructors -Wno-c++2a-compat -Wno-switch-enum -Wno-c++98-compat-pedantic -Wno-ctad-maybe-unsupported")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wpedantic -Wextra")
elseif(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus /W4 /wd4503 /wd4512 /utf-8")
endif()

include_directories (${CMAKE_BINARY_DIR})

add_custom_command(
   OUTPUT CafeBabe.c CafeBabe.h
   COMMAND packcc -o CafeBabe ../CafeBabe.peg
   DEPENDS CafeBabe.peg
)

add_compile_definitions(UNUSE_MAIN)
#enable_testing()

add_executable(cbc main.cpp ${CMAKE_BINARY_DIR}/CafeBabe.c ${CMAKE_BINARY_DIR}/CafeBabe.h)
target_link_libraries(cbc ${add_link_deps})
