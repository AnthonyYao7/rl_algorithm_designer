cmake_minimum_required(VERSION 3.23)

set(CMAKE_CXX_STANDARD 23)

add_executable(test testing_env_test.cpp)

target_include_directories(test PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_compile_options(test PUBLIC -g -Wall)