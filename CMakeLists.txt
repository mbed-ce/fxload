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

# Build executable
# ----------------------------------------------------------
set(FXLOAD_SOURCES
    ezusb.h
	ezusb.c
	main.cpp
	usb.h
	fxload-version.h
	${CMAKE_CURRENT_BINARY_DIR}/fxload-version.cpp)

# Set up version file
configure_file(fxload-version.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/fxload-version.cpp)

add_executable(fxload ${FXLOAD_SOURCES})
target_link_libraries(fxload libusb1::libusb1)
target_include_directories(fxload PRIVATE .)

# pick up CLI11 header only library
target_include_directories(fxload PRIVATE CLI11/include)