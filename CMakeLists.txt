cmake_minimum_required(VERSION 3.16)
project(english_learning_web LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  asio
  GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
  GIT_TAG asio-1-30-2
)
FetchContent_MakeAvailable(asio)
set(ASIO_INCLUDE_DIR "${asio_SOURCE_DIR}/asio/include" CACHE PATH "" FORCE)

FetchContent_Declare(
  crow
  GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
  GIT_TAG v1.2.0
)
FetchContent_MakeAvailable(crow)

find_package(SQLite3 REQUIRED)

add_executable(english_learning_web
  src/main.cpp
  src/database.cpp
  src/csv_parser.cpp
  src/vocabulary.cpp
)

target_include_directories(english_learning_web PRIVATE src)
target_link_libraries(english_learning_web PRIVATE Crow::Crow SQLite::SQLite3)
