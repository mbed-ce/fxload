cmake_minimum_required(VERSION 3.8)
cmake_policy(VERSION 3.8)

project(fxload VERSION 2.1)

# set compile flags and options
# ----------------------------------------------------------

# default to release build type
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Allowed build types." FORCE)
if("${CMAKE_BUILD_TYPE}" STREQUAL "")
	set(CMAKE_BUILD_TYPE RELEASE CACHE STRING "Type of build.  Controls debugging information and optimizations." FORCE)
endif()

# compile flags
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")

	set(CMAKE_CXX_FLAGS_RELEASE "-O3")
	set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
		
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")

	set(CMAKE_CXX_FLAGS_RELEASE "/O2 /MD")
	set(CMAKE_CXX_FLAGS_DEBUG "/MDd /Zi /Ob0 /Od /RTC1")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3") # Use reasonable warning flags

	# Suppress warning about zero sized arrays used by libusb
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4200")

	# disable verbose security warnings
	add_definitions(-D_CRT_SECURE_NO_WARNINGS=1)

elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang")

	set(CMAKE_CXX_FLAGS_RELEASE "-O3")
	set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
else()

	message(WARNING "Unknown compiler, don't know how to set CXXFLAGS")
endif()

# use C++17, with extensions
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS TRUE)

# Find dependencies
# ----------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

find_package(USB1 REQUIRED)

# CLI11 is a header only library
add_library(CLI11 INTERFACE)
target_include_directories(CLI11 INTERFACE CLI11/include)

# Add subdirs
# ----------------------------------------------------------
add_subdirectory(src)
add_subdirectory(resources)