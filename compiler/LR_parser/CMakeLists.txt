cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)

project(LR_parser)

add_executable(LR_parser main.cpp Grammar.cpp LR1Table.cpp)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

if(MSVC)
  target_compile_options(LR_parser PRIVATE /W4)
else()
  target_compile_options(LR_parser PRIVATE -Wall -Wextra)
endif()

