add_library(stun_message)

target_sources(stun_message
    PRIVATE
        src/error_code_attribute.cpp
        src/finger_print_attribute.cpp
        src/mapped_address_attribute.cpp
        src/stun_attribute_deserializer.cpp
        src/stun_message.cpp
        src/xor_mapped_address_attribute.cpp
)

target_include_directories(stun_message
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
