cmake_minimum_required(VERSION 3.15)

project(stun VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(STUN_ENABLE_COVERAGE "Build with coverage instrumentation" OFF)

add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wformat=2
    -Wnull-dereference
    -Werror
)

if(STUN_ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-O0 -g --coverage)
        add_link_options(--coverage)
    else()
        message(FATAL_ERROR "STUN_ENABLE_COVERAGE is supported only with GNU or Clang")
    endif()
endif()

add_subdirectory(stun_library)
add_subdirectory(client)
add_subdirectory(server)

enable_testing()
add_subdirectory(tests)
