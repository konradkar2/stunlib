add_executable(
    my_tests
    example_test.cpp
)

enable_testing()

find_package(GTest)

target_link_libraries(
    my_tests
    PRIVATE
    GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(my_tests)