# Plain-executable tests registered with CTest — no test framework dependency
# (mirrors the Python `if __name__ == "__main__"` self-tests). Each test prints
# a line and returns non-zero on failure.

add_executable(pattern_test pattern_test.cpp)
target_link_libraries(pattern_test PRIVATE procvd_core)
add_test(NAME pattern_test COMMAND pattern_test)

add_executable(geometry_test geometry_test.cpp)
target_link_libraries(geometry_test PRIVATE procvd_core)
add_test(NAME geometry_test COMMAND geometry_test)
