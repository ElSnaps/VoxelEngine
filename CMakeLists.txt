cmake_minimum_required(VERSION 3.13)
project(VoxelEngine)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_SUPPRESS_REGENERATION true)

add_subdirectory(Source)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VoxelEngine)