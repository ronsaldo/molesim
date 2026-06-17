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

include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/chemfiles/include"
)

add_subdirectory(thirdparty)

add_subdirectory(apps)

# Data files to distribute.
set(DATA_OUTPUT_PREFIX "${MainOutputDirectory}")
if(CMAKE_CONFIGURATION_TYPES)
    set(DATA_OUTPUT_PREFIX "${MainOutputDirectory}/$<CONFIG>")
endif()
