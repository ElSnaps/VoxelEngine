# Copyright Snaps 2022, All Rights Reserved.

############################################################################################################
# MACROS
############################################################################################################

# GroupSources
macro(GroupSources curdir)
    file(GLOB children RELATIVE ${PROJECT_SOURCE_DIR}/${curdir} ${PROJECT_SOURCE_DIR}/${curdir}/*)

    foreach(child ${children})
        if(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/${curdir}/${child})
            GroupSources(${curdir}/${child})
        else()
            string(REPLACE "/" "\\" groupname ${curdir})

            # I would like to call the src root folder in a different name, only in visual studio (not mandatory requirement)
            string(REPLACE "src" "Common" groupname ${groupname})

            source_group(${groupname} FILES ${PROJECT_SOURCE_DIR}/${curdir}/${child})
        endif()
    endforeach()
endmacro()

############################################################################################################
# CORE ENGINE MODULE SETUP
############################################################################################################

# Recursively find all source files within CoreEngine module
file(GLOB_RECURSE CoreEngineSrcs "*.c" "*.cpp" "*.h" "*.hpp")
add_executable(VoxelEngine ${CoreEngineSrcs})

#include_directories("C:\Program Files (x86)\Windows Kits\10\Lib\10.0.22000.0\um\x64")
#link_directories("C:\Program Files (x86)\Windows Kits\10\Lib\10.0.22000.0\um\x64")

#ADD_LIBRARY(CoreEngine 
#    d3d12.lib
#    dxgi.lib
#    d3dcompiler.lib
#)

# Link Libraries
#target_link_libraries(CoreEngine d3d12)
#target_link_libraries(CoreEngine dxgi)
#target_link_libraries(CoreEngine d3dcompiler)


# Setup filters in sln so that folders appear the same as in explorer
GroupSources(Source)
