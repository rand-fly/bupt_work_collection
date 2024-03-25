cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)

project(lexer)

add_executable(lexer main.cpp lexer.cpp)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

if(MSVC)
  target_compile_options(lexer PRIVATE /W4)
else()
  target_compile_options(lexer PRIVATE -Wall -Wextra)
endif()

