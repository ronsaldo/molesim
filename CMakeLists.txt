# Molesim cmake build script.
cmake_minimum_required(VERSION 3.10)

project(Molesim)

# Turn warnings
if (MSVC)
	# using Visual Studio C++
	#string(REGEX REPLACE "/W3" "/W4" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
	#string(REGEX REPLACE "/W3" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17")
endif()


set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/dist")

set(MainOutputDirectory "${Molesim_BINARY_DIR}/dist")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MainOutputDirectory} )
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${MainOutputDirectory} )
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${MainOutputDirectory} )

# For VisualStudio in Window
if(WIN32)
    set(SDL2_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/SDL2/include")
    set(SDL2_LIBRARY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/SDL2/lib/x64")
else()
    find_path(SDL2_INCLUDE_DIRS
    NAMES SDL.h
    PATH_SUFFIXES SDL2
    )

    find_path(SDL2_LIBRARY_PATH
    NAMES SDL2.lib SDL2.a SDL2-2.0.lib SDL2-2.0.a libSDL2-2.0.a
    )
endif()
    
find_library(SDL2_LIBRARY NAMES SDL2-2.0 SDL2 PATHS ${SDL2_LIBRARY_PATH})
find_library(SDL2_MAIN_LIBRARY NAMES SDL2main PATHS ${SDL2_LIBRARY_PATH})

if(SDL2_INCLUDE_DIRS AND SDL2_LIBRARY AND SDL2_INCLUDE_DIRS)
    # Use the SDL2 include path.
    include_directories(${SDL2_INCLUDE_DIRS})

	set(SDL2_FOUND TRUE)
	if(SDL2_MAIN_LIBRARY)
    	set(SDL2_LIBRARIES ${SDL2_LIBRARY} ${SDL2_MAIN_LIBRARY})
	else()
		set(SDL2_LIBRARIES ${SDL2_LIBRARY})
	endif()
endif()

include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/abstract-gpu/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/chemfiles/include"
)

add_subdirectory(thirdparty)

add_subdirectory(apps)

# Data files to distribute.
set(DATA_OUTPUT_PREFIX "${MainOutputDirectory}")
if(CMAKE_CONFIGURATION_TYPES)
    set(DATA_OUTPUT_PREFIX "${MainOutputDirectory}/$<CONFIG>")
endif()

if(NOT WIN32)
    file(GLOB ASSET_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
        "assets/shaders/*.glsl"
        "assets/textures/*.bmp"
    )

    add_custom_target(SampleData ALL)
    foreach(asset_file ${ASSET_FILES})
        add_custom_command(
            TARGET SampleData
            POST_BUILD
            COMMAND cmake -E copy
                "${CMAKE_CURRENT_SOURCE_DIR}/${asset_file}"
                "${DATA_OUTPUT_PREFIX}/${asset_file}"
            DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${asset_file}"
            COMMENT "Copy ${asset_file}"
        )
    endforeach()

endif()