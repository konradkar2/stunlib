add_executable(server)

target_sources(server
    PRIVATE
        main.cpp
        server.cpp
)

target_link_libraries(server
    PRIVATE
        stun_library
)
