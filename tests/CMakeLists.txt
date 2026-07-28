add_executable(
    my_tests
    error_code_attribute_test.cpp
    finger_print_attribute_test.cpp
    mapped_address_attribute_test.cpp
    stun_attribute_test.cpp
    stun_message_test.cpp
    xor_mapped_address_attribute_test.cpp
)

find_package(GTest)

target_link_libraries(
    my_tests
    PRIVATE
    stun_message
    GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(my_tests)
