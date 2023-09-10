cmake_minimum_required(VERSION 3.8)

set(CMAKE_C_STANDARD 17)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")

project(dnsrelay LANGUAGES C)

add_executable(dnsrelay "main.c" "socket.c" "data.c" "table.c" "id_convert.c" "log.c"
"$<$<C_COMPILER_ID:MSVC>:getopt.c>" )

if(MSVC)
  target_compile_options(dnsrelay PRIVATE /W4)
else()
  target_compile_options(dnsrelay PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(WIN32)
  target_link_libraries(dnsrelay ws2_32)
endif()
