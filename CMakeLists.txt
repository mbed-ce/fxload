cmake_minimum_required(VERSION 3.8)
cmake_policy(VERSION 3.8)

project(fxload VERSION 2.0)

# set compile flags and options
# ----------------------------------------------------------

# default to release build type
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Allowed build types." FORCE)
if("${CMAKE_BUILD_TYPE}" STREQUAL "")
	set(CMAKE_BUILD_TYPE RELEASE CACHE STRING "Type of build.  Controls debugging information and optimizations." FORCE)
endif()

option(USE_WERROR "If set to true, convert compiler warnings into compiler errors.  Mainly intended for CI." FALSE)

# compile flags
if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")

	set(CMAKE_C_FLAGS_RELEASE "-O3")
	set(CMAKE_C_FLAGS_DEBUG "-g3 -O0")
	string(APPEND CMAKE_C_FLAGS " -Wall")
	if(USE_WERROR)
		string(APPEND CMAKE_C_FLAGS " -Werror")
	endif()

elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")

	set(CMAKE_C_FLAGS_RELEASE "/O2 /MD")
	set(CMAKE_C_FLAGS_DEBUG "/MDd /Zi /Ob0 /Od /RTC1")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3") # Use reasonable warning flags

	# Suppress warning about zero sized arrays used by libusb
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4200")

	if(USE_WERROR)
		string(APPEND CMAKE_C_FLAGS " /WX")
	endif()

	# disable verbose security warnings
	add_definitions(-D_CRT_SECURE_NO_WARNINGS=1)

elseif(${CMAKE_C_COMPILER_ID} STREQUAL "Clang" OR ${CMAKE_C_COMPILER_ID} STREQUAL "AppleClang")

	set(CMAKE_C_FLAGS_RELEASE "-O3")
	set(CMAKE_C_FLAGS_DEBUG "-g3 -O0")
	string(APPEND CMAKE_C_FLAGS " -Wall")

	if(USE_WERROR)
		string(APPEND CMAKE_C_FLAGS " -Werror")
	endif()

else()

	message(WARNING "Unknown compiler, don't know how to set CXXFLAGS")
endif()


if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")

	set(CMAKE_CXX_FLAGS_RELEASE "-O3")
	set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0")
	string(APPEND CMAKE_CXX_FLAGS " -Wall")
	if(USE_WERROR)
		string(APPEND CMAKE_CXX_FLAGS " -Werror")
	endif()
		
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")

	set(CMAKE_CXX_FLAGS_RELEASE "/O2 /MD")
	set(CMAKE_CXX_FLAGS_DEBUG "/MDd /Zi /Ob0 /Od /RTC1")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3") # Use reasonable warning flags

	# Suppress warning about zero sized arrays used by libusb
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4200")

	if(USE_WERROR)
		string(APPEND CMAKE_CXX_FLAGS " /WX")
	endif()

elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang")

	set(CMAKE_CXX_FLAGS_RELEASE "-O3")
	set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

	if(USE_WERROR)
		string(APPEND CMAKE_CXX_FLAGS " -Werror")
	endif()

else()

	message(WARNING "Unknown compiler, don't know how to set CXXFLAGS")
endif()

# use C99 and C++17, with extensions
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS TRUE)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS TRUE)

# Find dependencies
# ----------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

find_package(USB1 REQUIRED)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/CLI11/LICENSE")
	message(FATAL_ERROR "CLI11 submodule missing, please run 'git submodule update --init'")
endif()

# CLI11 is a header only library
add_library(CLI11 INTERFACE)
target_include_directories(CLI11 INTERFACE CLI11/include)

# Add subdirs
# ----------------------------------------------------------
add_subdirectory(src)
add_subdirectory(resources)

# Set up packaging (on windows)
# ----------------------------------------------------------
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
	# General CPack options
	set(CPACK_PACKAGE_NAME "FXLoad")
	set(CPACK_PACKAGE_VENDOR "Mbed CE")
	set(CPACK_PACKAGE_DESCRIPTION "FXLoad is a tool for flashing firmware to the EZ-USB series of USB interface microcontrollers.")
	set(CPACK_PACKAGE_VERSION ${fxload_VERSION})
	set(CPACK_PACKAGE_INSTALL_DIRECTORY "FXLoad")
	set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/mbed-ce/fxload")
	set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/copying.txt)

	# NSIS specific options
	set(CPACK_GENERATOR NSIS)
	# Make sure the installer provides an option to add fxload to the path
	set(CPACK_NSIS_MODIFY_PATH TRUE)

	include(CPack)
endif()