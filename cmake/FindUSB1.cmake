# - Try to find the libusb library
# Once done this defines
#
#  USB1_FOUND - system has libusb
#  USB1_INCLUDE_DIR - the libusb include directory (contains libusb-1.0/libusb.h)
#  USB1_LIBRARIES - Link these to use libusb
#
# If USB1_FOUND is true, it also creates the following imported target:
#  libusb1::libusb1

# Copyright (c) 2006, 2008  Laurent Montel, <montel@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  find_package(PkgConfig)
  pkg_check_modules(PC_USB1 libusb-1.0)
endif()

set(USB1_LIBRARY_NAMES usb-1.0)
if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
  set(USB1_LIBRARY_NAMES usb)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")

  # vcpkg's libusb-1.0 has a "lib" prefix, but on Windows MVSC, CMake doesn't search for
  # static libraries with lib prefixes automatically.
  list(APPEND USB1_LIBRARY_NAMES libusb-1.0)
endif()


find_path(USB1_INCLUDE_DIR libusb.h
  PATHS ${PC_USB1_INCLUDEDIR} ${PC_USB1_INCLUDE_DIRS}
  PATH_SUFFIXES libusb-1.0)

find_library(USB1_LIBRARIES NAMES ${USB1_LIBRARY_NAMES}
  PATHS ${PC_USB1_LIBDIR} ${PC_USB1_LIBRARY_DIRS})
  
mark_as_advanced(USB1_INCLUDE_DIR USB1_LIBRARIES USB1_WORKS)

include(CMakePushCheckState)
include(CheckFunctionExists)
cmake_push_check_state()
set(CMAKE_REQUIRED_LIBRARIES ${USB1_LIBRARIES})
check_function_exists(libusb_init USB1_WORKS)
cmake_pop_check_state()
	
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(USB1 DEFAULT_MSG USB1_LIBRARIES USB1_INCLUDE_DIR USB1_WORKS)

if(USB1_FOUND)
    add_library(libusb1::libusb1 UNKNOWN IMPORTED)
    set_property(TARGET libusb1::libusb1 PROPERTY IMPORTED_LOCATION ${USB1_LIBRARIES})
    set_property(TARGET libusb1::libusb1 PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${USB1_INCLUDE_DIR})
endif()