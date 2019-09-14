include(CTest)

add_executable(test_single_thread single-thread-tests.cpp)
target_compile_features(test_single_thread PRIVATE cxx_std_17)
target_link_libraries(test_single_thread PRIVATE sqlite_wrapper)
add_test (single_thread test_single_thread)