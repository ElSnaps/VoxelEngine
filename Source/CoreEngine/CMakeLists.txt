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
# MODULE SETUP
############################################################################################################

# Recursively find all source files within CoreEngine module
file(GLOB_RECURSE CoreEngineSrcs "*.c" "*.cpp" "*.h" "*.hpp")
add_executable(VoxelEngine ${CoreEngineSrcs})

# Setup filters in sln so that folders appear the same as in explorer
GroupSources(Source)