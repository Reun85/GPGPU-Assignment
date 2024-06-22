cmake_minimum_required(VERSION 3.27.0)


set(PROJECT_NAME "GPGPU")

set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")


# Hozzá ad egy flaget amiből lehet tudni, hogy DEBUG módban indítottuk el a programot.
set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -g")

set(CMAKE_CXX_FLAGS "-Wall -pedantic ")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

include_directories(src/includes)


set(OPENCL_VENDOR_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/vendor/OpenCL")



project(${PROJECT_NAME} VERSION 0.1.0)


set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(DESTINATION_DIR "${CMAKE_CURRENT_BINARY_DIR}")

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "${SOURCE_DIR}/*.cpp" "${SOURCE_DIR}/*.h" "${SOURCE_DIR}/*.c" "${SOURCE_DIR}/*.hpp")
file(GLOB_RECURSE GLSL_FILES CONFIGURE_DEPENDS "${SOURCE_DIR}/*.glsl" "${SOURCE_DIR}/*.vert" "${SOURCE_DIR}/*.frag")
file(GLOB_RECURSE ASSETS CONFIGURE_DEPENDS "${SOURCE_DIR}/Assets/**")

function(copy_files_relative_to_src)
  foreach(FILE ${ARGN})
    # Extract the relative path of the file from the source directory
    file(RELATIVE_PATH REL_PATH ${SOURCE_DIR} ${FILE})
    # Construct the destination path
    set(DEST_PATH "${DESTINATION_DIR}/${REL_PATH}")
    # Create the necessary directories in the destination
    get_filename_component(DEST_DIR "${DEST_PATH}" DIRECTORY)
    file(MAKE_DIRECTORY "${DEST_DIR}")
    # Copy the file to the destination
    file(COPY "${FILE}" DESTINATION "${DEST_DIR}")
  endforeach()
endfunction()

copy_files_relative_to_src(${ASSETS})
copy_files_relative_to_src(${GLSL_FILES})

include_directories(${SOURCE_DIR})
add_executable(${PROJECT_NAME} ${SOURCE_FILES})


# Könyvtárak
find_package(GLEW CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE GLEW::GLEW)


find_package(glm CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui::imgui)

# find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME}
        PRIVATE
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    )

find_package(SDL2_image CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>)

find_package(OpenCL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenCL::OpenCL)

include_directories(vendor)
# add_subdirectory(vendor/OpenCL)
# #
# target_link_libraries(${PROJECT_NAME} PRIVATE OpenCLLib "-std=c++11")
