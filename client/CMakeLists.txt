add_executable(client)

target_sources(client
    PRIVATE
        main.cpp
        client.cpp
)

target_link_libraries(client
    PRIVATE
        stun_library
)

target_include_directories(client
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
