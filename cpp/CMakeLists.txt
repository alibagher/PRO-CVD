cmake_minimum_required(VERSION 3.21)
project(procvd LANGUAGES CXX VERSION 0.1.0)

# C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# no extensions
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to Release if the user didn't pick a build type.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

#  OpenCV is the only external dependency
find_package(OpenCV REQUIRED)

# pattern.cpp / geometry.cpp are  data in, data out. The rest is the hardware shell.
# Kept in one library for simplicity
add_library(procvd_core
  src/pattern.cpp
  src/geometry.cpp
  src/camera.cpp
  src/projector.cpp
  src/app.cpp
)
target_include_directories(procvd_core PUBLIC include)
target_link_libraries(procvd_core PUBLIC ${OpenCV_LIBS})
target_include_directories(procvd_core SYSTEM PUBLIC ${OpenCV_INCLUDE_DIRS})

# private: options are used only by this target, not by anything that links to it
# MSVC: Set to true when the compiler is some version of Microsoft Visual C++ (Windows)
if(MSVC)
  target_compile_options(procvd_core PRIVATE /W4)
else()
  target_compile_options(procvd_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Runtime executable ----------------------------------------------------
add_executable(procvd src/main.cpp)
target_link_libraries(procvd PRIVATE procvd_core)

# Tests (run with: ctest)
enable_testing()
add_subdirectory(tests)
