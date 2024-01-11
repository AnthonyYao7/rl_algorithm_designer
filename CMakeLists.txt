cmake_minimum_required(VERSION 3.23)
project(codemealife)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(tests/trie_tests)
add_subdirectory(src)
add_subdirectory(testing)
add_subdirectory(src/testing_harness)