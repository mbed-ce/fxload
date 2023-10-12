set(FXLOAD_SOURCES
    ezusb.h
	ezusb.c
	main.cpp
	ApplicationPaths.cpp
	ApplicationPaths.h
	fxload-version.h
	${CMAKE_CURRENT_BINARY_DIR}/fxload-version.cpp)

# Set up version file
configure_file(fxload-version.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/fxload-version.cpp)

add_executable(fxload ${FXLOAD_SOURCES})
target_link_libraries(fxload libusb1::libusb1 CLI11)
target_include_directories(fxload PRIVATE .)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
	# On Windows we need Shlwapi.lib for PathRemoveFileSpecA
	target_link_libraries(fxload Shlwapi)
endif()

install(TARGETS fxload DESTINATION bin)