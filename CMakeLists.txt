cmake_minimum_required(VERSION 3.15)
project(stun LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(
-Wall
-Wextra
-Wpedantic
-Wconversion
-Wshadow
-Wformat=2
-Wduplicated-branches
-Wduplicated-cond
-Wnull-dereference
-Werror
)

add_executable(${PROJECT_NAME} main.cpp)
