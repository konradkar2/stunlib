cmake_minimum_required(VERSION 3.15)
project(stun LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(
-Wall
-Wextra
-Wpedantic
#-Wconversion
-Wshadow
-Wformat=2
-Wnull-dereference
-Werror
)

add_library(stun_lib stun_message.cpp stun_attributes.cpp)
target_include_directories(stun_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(${PROJECT_NAME} main.cpp )
target_link_libraries(${PROJECT_NAME} PRIVATE stun_lib)

enable_testing()
add_subdirectory(tests)
