cmake_minimum_required(VERSION 3.16)
project(CafeBabe)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(./cpp-peglib)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything -Wno-c++98-compat -Wno-padded -Wno-weak-vtables -Wno-exit-time-destructors -Wno-c++2a-compat -Wno-switch-enum -Wno-c++98-compat-pedantic -Wno-ctad-maybe-unsupported")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wpedantic -Wextra")
elseif(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus /W4 /wd4503 /wd4512 /utf-8")
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(add_link_deps Threads::Threads)
endif()

add_custom_command(
   OUTPUT grammar.cpp
   COMMAND ../assemble.sh > ${CMAKE_CURRENT_BINARY_DIR}/grammar.cpp
   DEPENDS CafeBabe.peg
)

#add_subdirectory(test)
#enable_testing()

add_executable(cbc main.cpp cpp-peglib/peglib.h ${CMAKE_CURRENT_BINARY_DIR}/grammar.cpp)
target_link_libraries(cbc ${add_link_deps})
