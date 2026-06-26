target_sources(stun_lib PRIVATE
    error_code_attribute.cpp
    finger_print_attribute.cpp
    mapped_address_attribute.cpp
    stun_attribute_deserializer.cpp
    stun_message.cpp
    xor_mapped_address_attribute.cpp
)

target_include_directories(stun_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)