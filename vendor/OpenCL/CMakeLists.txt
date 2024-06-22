cmake_minimum_required(VERSION 3.10)
project(OpenCLLib VERSION 1.0)


# Include directory for header files
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Gather header files
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)

# Create the library target
add_library(OpenCLLib STATIC m.cpp)



# Set the include directories
target_include_directories(OpenCLLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Optionally, specify compiler options or standards if needed
# For example, setting C++14 standard
target_compile_features(OpenCLLib PRIVATE cxx_std_14)
# Or specific compile definitions

# Optionally, you can also set properties like C++ standard requirements
# set(CMAKE_CXX_FLAGS "-std=c++11")
# set_target_properties(OpenCLLib PROPERTIES
#     CXX_STANDARD 14
#     CXX_STANDARD_REQUIRED ON
# )
