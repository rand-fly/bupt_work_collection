cmake_minimum_required (VERSION 3.14)

set(CMAKE_CXX_STANDARD 17)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")


# --- Fetch FTXUI --------------------------------------------------------------
include(FetchContent)
 
set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
set(FTXUI_BUILD_EXAMPLES OFF)
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG v4.0.0
)
FetchContent_MakeAvailable(ftxui)
 
# ------------------------------------------------------------------------------

FetchContent_Declare(asio
  GIT_REPOSITORY https://github.com/chriskohlhoff/asio
  GIT_TAG master
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
)
FetchContent_MakeAvailable(asio)

add_library(asio INTERFACE)
target_include_directories(asio INTERFACE ${asio_SOURCE_DIR}/asio/include)

project(client
  LANGUAGES CXX
  VERSION 1.0.0
)
 
add_executable(client
	client/main.cpp
	client/LoginPage.cpp
	client/SignupPage.cpp
	client/ConnectPage.cpp
	client/Socket.cpp
	client/MainPage.cpp
 "client/MakePage.cpp" "client/PlayPage.cpp" "client/RankPage.cpp"  "client/BattlePage.cpp")
target_include_directories(client PRIVATE client common)
 
target_link_libraries(client
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
  PRIVATE asio
)

project(server
  LANGUAGES CXX
  VERSION 1.0.0
)
 
add_executable(server
	server/main.cpp
	server/User.cpp
	server/Database.cpp
	server/Session.cpp
   "server/Battle.h" "server/Battle.cpp")
target_include_directories(server PRIVATE server common)

target_link_libraries(server
  PRIVATE asio
)