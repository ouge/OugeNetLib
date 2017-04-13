cmake_minimum_required(VERSION 3.5.0)

set (PROJECT_NAME OugeNetlib)
project(${PROJECT_NAME})


enable_testing()


set(CXX_FLAGS
 -g
 -Wall
 -Wextra
 -Werror
 -Wconversion
 -Wno-unused-parameter
 -Wold-style-cast
 -Woverloaded-virtual
 -Wpointer-arith
 -Wshadow
 -Wwrite-strings
 -march=native
 -MMD
 -std=c++1z
 -rdynamic
 )




set(CMAKE_C_COMPILER "clang")  
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)




add_subdirectory(base)
add_subdirectory(gtest)
