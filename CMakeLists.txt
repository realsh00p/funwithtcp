cmake_minimum_required(VERSION 3.18)

project(
  funwithtcp
)

find_package(Threads REQUIRED COMPONENTS Threads)

set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")
set(CMAKE_CXX_STANDARD 20)

add_library(
  lib INTERFACE
)

target_sources(
  lib
  INTERFACE
    src/include/EventLoop.hpp
)

target_include_directories(
  lib
  INTERFACE
    src/include
)

find_package(Boost)

target_link_libraries(
  lib
  INTERFACE
    Boost::boost
    Threads::Threads
)

add_executable(
  test
)

target_sources(
  test
  PRIVATE
    test/main.cpp
)

target_link_libraries(
  test
  PRIVATE
    lib
)
