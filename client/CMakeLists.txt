target_sources(stun_lib PRIVATE
    client.cpp
)

target_include_directories(stun_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
