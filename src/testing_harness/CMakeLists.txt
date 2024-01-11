cmake_minimum_required(VERSION 3.23)

set(CMAKE_CXX_STANDARD 23)

add_executable(ctester ctester.c example_testing_harness.c)

target_include_directories(ctester PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_compile_options(ctester PUBLIC -g -Wall)