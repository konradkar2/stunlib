add_library(stun_library)
add_library(stun::stun_library ALIAS stun_library)

include(GNUInstallDirs)

target_sources(stun_library
    PRIVATE
        src/error_code_attribute.cpp
        src/finger_print_attribute.cpp
        src/ip_address.cpp
        src/mapped_address_attribute.cpp
        src/stun_attribute_deserializer.cpp
        src/stun_message.cpp
        src/xor_mapped_address_attribute.cpp
)

target_include_directories(stun_library
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

install(
    TARGETS stun_library
    EXPORT stunConfig
)

install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    EXPORT stunConfig
    NAMESPACE stun::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/stun
)
