# Copyright Snaps 2022, All Rights Reserved.

############################################################################################################
# GLOBAL SETUP
############################################################################################################

# Specify CMake Project
cmake_minimum_required(VERSION 3.13)
project(VoxelEngine)

# Set C++14 to default
set_property(GLOBAL PROPERTY CMAKE_CXX_STANDARD 14)

# Set Global Properties
set(CMAKE_SUPPRESS_REGENERATION true)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Binaries)
set_property(GLOBAL PROPERTY PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/Source)

# Set default sln project to VoxelEngine
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VoxelEngine)

# Main Source Directory
add_subdirectory(Source)