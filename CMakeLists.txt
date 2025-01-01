cmake_minimum_required(VERSION 3.31.3 FATAL_ERROR)
project("Iridium" LANGUAGES C VERSION 0.0.1 DESCRIPTION "A 3D game engine written in C23.")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/Modules")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Iridium/Demos)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Iridium/Library)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Iridium/Library)

option(BUILD_SHARED_LIBS "Build Iridium as a dynamic library." ON)
option(IRIDIUM_BUILD_DEMOS "Build the Iridium demo programs." ON)
option(IRIDIUM_NO_SANITIZE "Don't sanitize output code (Debug)." OFF)
# Unimplemented.
option(IRIDIUM_BUILD_DOCS "Build the Iridium documentation." ON)

find_package(Vulkan REQUIRED)
find_package(Threads REQUIRED)
if(LINUX)
    # We do not yet deal in the devilish magic of X11.
    find_package(Wayland REQUIRED)
endif()

set(CMAKE_C_STANDARD_REQUIRED true)
set(CMAKE_C_STANDARD 23)

add_compile_options(-Wall -Werror -Wpedantic)
if(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_compile_options(-Ofast)
    add_compile_definitions(RELEASE_MODE=true)
else()
    # Provide several IDEs with code completion and such.
    set(CMAKE_EXPORT_COMPILE_COMMANDS true)
    add_compile_options(-ggdb3)
    # Valgrind does not play nice with UBSAN & ASAN.
    if(NOT IRIDIUM_NO_SANITIZE)
        add_compile_options(-fsanitize=address -fsanitize=undefined)
        add_link_options(-fsanitize=address -fsanitize=undefined)
    endif()
    add_compile_definitions(RELEASE_MODE=false)
endif()

set(IRIDIUM_SOURCE_DIR "${CMAKE_SOURCE_DIR}/Source")
set(IRIDIUM_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/Include")
set(IRIDIUM_DEMO_DIR "${CMAKE_SOURCE_DIR}/Demos")
include_directories("${IRIDIUM_INCLUDE_DIR}")

set(IRIDIUM_HEADER_FILES "${IRIDIUM_INCLUDE_DIR}/Iridium.h"
    "${IRIDIUM_INCLUDE_DIR}/Logging.h" "${IRIDIUM_INCLUDE_DIR}/Errors.h")
set(IRIDIUM_SOURCE_FILES "${IRIDIUM_SOURCE_DIR}/Iridium.c" 
    "${IRIDIUM_SOURCE_DIR}/Logging.c" "${IRIDIUM_SOURCE_DIR}/Errors.c")
if(LINUX)
    set(IRIDIUM_SOURCE_FILES ${IRIDIUM_SOURCE_FILES} 
        "${IRIDIUM_SOURCE_DIR}/Internal/Wayland.c" 
        "${IRIDIUM_SOURCE_DIR}/Internal/XDGShell.c")
endif()

# Loop over each source file and add a filename signature to it.
# We use this for logging purposes.
file(GLOB IRIDIUM_PROJECT_FILES ${IRIDIUM_SOURCE_FILES} ${IRIDIUM_HEADER_FILES})
foreach(file ${IRIDIUM_PROJECT_FILES})
    cmake_path(GET file FILENAME FILE_NAME)
    set_source_files_properties(${file} PROPERTIES COMPILE_DEFINITIONS 
        -DFILENAME="${FILE_NAME}")
endforeach()

if(BUILD_SHARED_LIBS)
    add_library(Iridium SHARED ${IRIDIUM_PROJECT_FILES})
    set_target_properties(Iridium PROPERTIES VERSION ${PROJECT_VERSION})
    set_target_properties(Iridium PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
else()
    add_library(Iridium STATIC ${IRIDIUM_PROJECT_FILES})
endif()

target_link_libraries(Iridium PRIVATE Vulkan::Vulkan Threads::Threads)
if(LINUX)
    add_compile_definitions(IRIDIUM_WAYLAND=true)
    target_link_libraries(Iridium PRIVATE Wayland::Wayland)
endif()

# Copy public API to the build directory.
file(COPY ${IRIDIUM_INCLUDE_DIR} DESTINATION ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

if(IRIDIUM_BUILD_DEMOS)
    # Loop through each C file in the Iridium demo directory and build it
    # as a demo.
    file(GLOB IRIDIUM_DEMO_FILES ${IRIDIUM_DEMO_DIR}/*.c)
    foreach(file ${IRIDIUM_DEMO_FILES})
        cmake_path(GET file STEM DEMO_FILE_STEM)
        cmake_path(GET file EXTENSION DEMO_FILE_EXTENSION)
        add_executable(${DEMO_FILE_STEM} ${file})
        target_link_libraries(${DEMO_FILE_STEM} Iridium)
        set_target_properties(${DEMO_FILE_STEM} PROPERTIES LINK_FLAGS "-Wl,-rpath,./")
        # Add our nifty little filename maco to each demo file.
        target_compile_definitions(${DEMO_FILE_STEM} PRIVATE 
            FILENAME="${DEMO_FILE_STEM}${DEMO_FILE_EXTENSION}")
    endforeach()
endif()