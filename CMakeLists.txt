cmake_minimum_required(VERSION 3.19)

if(" ${CMAKE_SOURCE_DIR}" STREQUAL " ${CMAKE_BINARY_DIR}")
	message(FATAL_ERROR "In-source builds are strictly prohibited. Please see BUILDING.md and use that CMake command line as an example.")
endif()

include(cmake/Policies.cmake)

project(bfsxtract)


set(CMAKE_CXX_STANDARD 20)

add_subdirectory(third_party/ELFIO)
add_subdirectory(src)
