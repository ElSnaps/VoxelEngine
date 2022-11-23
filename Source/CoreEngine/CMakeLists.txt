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
add_executable(VoxelEngine WIN32 ${CoreEngineSrcs})

target_include_directories(VoxelEngine
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/Source/ThirdParty/SDL2/Include
)

target_link_directories(VoxelEngine
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/Source/ThirdParty/SDL2/Lib
)

target_link_libraries(VoxelEngine
    PRIVATE 
        SDL2.lib
        SDL2
)

# Copy SDL dll next to .exe
if(WIN32)
    add_custom_command(
        TARGET VoxelEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        ${PROJECT_SOURCE_DIR}/Source/ThirdParty/SDL2/Lib/SDL2.dll $<TARGET_FILE_DIR:VoxelEngine> 
        COMMAND_EXPAND_LISTS
    )
endif()

# Setup filters in sln so that folders appear the same as in explorer
GroupSources(Source)
