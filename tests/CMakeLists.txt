add_executable(
    my_tests
    example_test.cpp
    stun_attributes_test.cpp
)

find_package(GTest)

target_link_libraries(
    my_tests
    PRIVATE
    stun_lib
    GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(my_tests)
