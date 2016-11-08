cmake_minimum_required(VERSION 3.0)
project(liblogger)

set(CMAKE_C_FLAGS           "${CMAKE_C_FLAGS}           -std=c89 -Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG     "${CMAKE_C_FLAGS_DEBUG}     -O0")
set(CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE}   -O3")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

#####
# Project structure
###
set(      PROJECT_PATH       "${PROJECT_SOURCE_DIR}")
set(      SOURCE_PATH        "${PROJECT_PATH}/src")
file(GLOB SOURCE_FILES       "${SOURCE_PATH}/*.c")

set(      DEPS_PATH          "${PROJECT_PATH}/deps")
set(      TEST_PATH          "${PROJECT_PATH}/test")
set(      EXAMPLE_PATH       "${PROJECT_PATH}/examples")

#####
# Dependencies
###
set(DEPS_LIST "")
function(add_dependency TARGET PATH)
    file(GLOB FILES "${PATH}/*.c" "${PATH}/*.h")
    add_library("${TARGET}" "${FILES}")
    set(DEPS_LIST ${DEPS_LIST} ${TARGET} PARENT_SCOPE)
endfunction()
include_directories("${DEPS_PATH}")

add_dependency(ansicolor-w32 "${DEPS_PATH}/ansicolor-w32")
add_dependency(extname "${DEPS_PATH}/extname")

#####
# Library
###
add_library(logger "${SOURCE_FILES}")
target_link_libraries(logger "${DEPS_LIST}")

#####
# Examples
###
option(BUILD_EXAMPLES "Build examples" ON)

if (BUILD_EXAMPLES)
    add_executable(example "${EXAMPLE_PATH}/example.c")
    target_include_directories(example PRIVATE "${SOURCE_PATH}")
    target_link_libraries(example logger)
endif()
