# This generation file is a very early version. It only builds a shared
# library and doesn't provide configuration options.

cmake_minimum_required(VERSION 3.30 FATAL_ERROR)
project(Iridium VERSION 1.1.0 LANGUAGES CXX C DESCRIPTION "A 3D game engine built in C++17.")
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

# Make sure the project is compiled via the correct C++ version.
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_STANDARD 20)

add_compile_options(-Wall -Wpedantic -Werror)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-Ofast)
else()
    add_compile_options(-g -fsanitize=address -fsanitize=undefined -fsanitize=leak)
    add_link_options(-fsanitize=address)
endif()

# Set all proper directories and note down locations.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Iridium/Demos)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Iridium/Library)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Iridium/Library)

set(IRIDIUM_SOURCE_DIR "${CMAKE_SOURCE_DIR}/Source")
set(IRIDIUM_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/Include")
set(IRIDIUM_DEMO_DIR "${CMAKE_SOURCE_DIR}/Demos")
include_directories(${IRIDIUM_INCLUDE_DIR})

# Note down all the files this project contains.
set(IRIDIUM_HEADER_FILES "${IRIDIUM_INCLUDE_DIR}/Version.hpp" "${IRIDIUM_INCLUDE_DIR}/Files.hpp" 
    "${IRIDIUM_INCLUDE_DIR}/Window.hpp" "${IRIDIUM_INCLUDE_DIR}/Logging.hpp" 
    "${IRIDIUM_INCLUDE_DIR}/Errors.hpp")
set(IRIDIUM_SOURCE_FILES "${IRIDIUM_SOURCE_DIR}/Version.cpp" "${IRIDIUM_SOURCE_DIR}/Files.cpp"
    "${IRIDIUM_SOURCE_DIR}/Window.cpp" "${IRIDIUM_SOURCE_DIR}/Wayland/Wayland.cpp"
    "${IRIDIUM_SOURCE_DIR}/Logging.cpp" "${IRIDIUM_SOURCE_DIR}/Errors.cpp")

# Create the Iridium library.
add_library(Iridium SHARED ${IRIDIUM_SOURCE_FILES})
set_target_properties(Iridium PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(Iridium PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
target_link_libraries(Iridium wayland-client)

# Create Demo Executables
file(GLOB IRIDIUM_DEMO_FILES ${IRIDIUM_DEMO_DIR}/*.cpp)
foreach(file ${IRIDIUM_DEMO_FILES})
    cmake_path(GET file STEM DEMO_FILE_STEM)
    add_executable(${DEMO_FILE_STEM} ${file})
    target_link_libraries(${DEMO_FILE_STEM} Iridium)
    set_target_properties(${DEMO_FILE_STEM} PROPERTIES LINK_FLAGS "-Wl,-rpath,./")
endforeach()

