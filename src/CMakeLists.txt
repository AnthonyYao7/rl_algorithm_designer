cmake_minimum_required(VERSION 3.23)

set(CMAKE_CXX_STANDARD 23)

add_executable(main main.cpp rl_environment/CodingEnvironment.cpp)

target_include_directories(main PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_compile_options(main PUBLIC -g -Wall)
