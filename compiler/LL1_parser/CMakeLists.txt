cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)

project(LL1_parser)

add_executable(LL1_parser main.cpp Grammar.cpp LL1Table.cpp)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

if(MSVC)
  target_compile_options(LL1_parser PRIVATE /W4)
else()
  target_compile_options(LL1_parser PRIVATE -Wall -Wextra)
endif()

